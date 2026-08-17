/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/
#include "waterout_daily.h"
#include "output.h"
#include "model.h"
#include "watercycle.h"
#include "resourceunit.h"
#include "climate.h"
#include "permafrost.h"
#include "debugtimer.h"

#include <QtCore/QMap>
#include <cmath>

namespace {
struct CellAccumulator
{
    int day = 0;
    int month = 0;
    int xCenter = 0;
    int yCenter = 0;
    int ru_count = 0;
    double weight = 0.0;
    double sumStockedArea = 0.0;
    double sumStockableArea = 0.0;
    double sumPrecipitation = 0.0;
    double sumTemp = 0.0;
    double sumEt = 0.0;
    double sumLaiTotal = 0.0;
    double sumLaiBroad = 0.0;
    double sumLaiConifer = 0.0;
    double sumSwc = 0.0;
    double sumPwp = 0.0;
};
}

DailyWaterOut::DailyWaterOut()
{
    setName("Daily output of AET and LAI", "water_day");
    setDescription("Daily water cycle output on resource unit/landscape unit.\n");
    columns()   << OutputColumn::ru()
                << OutputColumn::id()
                << OutputColumn::year()
                << OutputColumn("month",  "month of year (1..12)",     OutInteger)
                << OutputColumn("day",    "day of year (1..365/366)",  OutInteger)
                << OutputColumn("x_m", "x-coordinate of resource unit center (m)", OutDouble)
                << OutputColumn("y_m", "y-coordinate of resource unit center (m)", OutDouble)
                << OutputColumn("stocked_area", "area (ha/ha) which is stocked (covered by crowns, absorbing radiation)", OutDouble)
                << OutputColumn("stockable_area", "area (ha/ha) which is stockable (and within the project area)", OutDouble)
                << OutputColumn("precipitation_mm", "Daily precipitation sum (mm)", OutDouble)
                << OutputColumn("temp", "Daily mean temperature (°C)", OutDouble)
                << OutputColumn("et_mm", "Actual evapotranspiration (mm)", OutDouble)
                << OutputColumn("lai_total", "effective LAI (m2/m2) of all species including LAI of adult trees, saplings, and ground cover", OutDouble)
                << OutputColumn("lai_broadleaf", "effective LAI (m2/m2) of broadleaf trees only", OutDouble)
                << OutputColumn("lai_coniferous", "effective LAI (m2/m2) of conifer trees only", OutDouble)
                << OutputColumn("swc_mm", "soil water content of the day (mm)", OutDouble)
                << OutputColumn("pwp_mm", "permanent wilting point of the soil of the day (mm)", OutDouble);
}

void DailyWaterOut::exec()
{
    Model *m = GlobalSettings::instance()->model();
    const bool aggregateTo1km = settings().valueBool("output.water_day.aggregateTo1km", false);
    double computeMs = 0.;
    double outputMs = 0.;
    qint64 rowsWritten = 0;
    TickTack phaseTimer;

    // global condition
    if (!mCondition.isEmpty() && mCondition.calculate(GlobalSettings::instance()->currentYear())==0.)
        return;

    if (aggregateTo1km) {
        QMap<QString, CellAccumulator> cells;
        phaseTimer.start();

        foreach(ResourceUnit *ru, m->ruList()) {
            if (ru->id()==-1)
                continue; // do not include if out of project area

            const WaterCycle *wc = ru->waterCycle();
            const ClimateDay *day = ru->climate()->begin();
            const int n_days = ru->climate()->daysOfYear();
            const QPointF center = ru->boundingBox().center();
            const int xCell = static_cast<int>(std::floor(center.x() / 1000.0));
            const int yCell = static_cast<int>(std::floor(center.y() / 1000.0));
            const int xCenter = xCell * 1000 + 500;
            const int yCenter = yCell * 1000 + 500;
            const double weight = ru->stockableArea();

            for (int doy = 0; doy < n_days; ++doy, ++day) {
                const QString key = QString("%1:%2:%3").arg(xCell).arg(yCell).arg(doy + 1);
                CellAccumulator &acc = cells[key];

                acc.ru_count++;
                acc.day = doy + 1;
                acc.month = day->month;
                acc.weight += weight;
                acc.sumStockedArea += (ru->stockedArea()/cRUArea);// * weight;
                acc.sumStockableArea += (ru->stockableArea()/cRUArea);// * weight;
                acc.sumPrecipitation += day->preciptitation;// * weight;
                acc.sumTemp += day->temperature;// * weight;
                acc.sumEt += wc->dailyEvapotranspiration(doy);// * weight;
                acc.sumLaiTotal += wc->dailyEffectiveLAI(doy);// * weight;
                acc.sumLaiBroad += wc->dailyBroadleafLAI(doy);// * weight;
                acc.sumLaiConifer += wc->dailyConiferousLAI(doy);// * weight;
                acc.sumSwc += wc->dailyWaterContent(doy);// * weight;
                acc.sumPwp += wc->dailyPWP(doy);// * weight;
                acc.xCenter = xCenter;
                acc.yCenter = yCenter;
            }
        }

        computeMs = phaseTimer.elapsed() * 1000.;
        phaseTimer.start();

        for (QMap<QString, CellAccumulator>::const_iterator it = cells.constBegin(); it != cells.constEnd(); ++it) {
            const CellAccumulator &acc = it.value();
            //const double invWeight = acc.weight > 0.0 ? 1.0 / acc.weight : 0.0;

            *this << -1
                  << -1
                  << currentYear()
                  << acc.month
                  << acc.day
                  << acc.xCenter
                  << acc.yCenter
                  << acc.sumStockedArea / acc.ru_count//* invWeight
                  << acc.sumStockableArea / acc.ru_count//* invWeight
                  << acc.sumPrecipitation / acc.ru_count//* invWeight
                  << acc.sumTemp / acc.ru_count//* invWeight
                  << acc.sumEt / acc.ru_count//* invWeight
                  << acc.sumLaiTotal / acc.ru_count//* invWeight
                  << acc.sumLaiBroad / acc.ru_count//* invWeight
                  << acc.sumLaiConifer / acc.ru_count//* invWeight
                  << acc.sumSwc / acc.ru_count//* invWeight
                  << acc.sumPwp / acc.ru_count;//* invWeight;
            writeRow();
            ++rowsWritten;
        }

        outputMs = phaseTimer.elapsed() * 1000.;
        qDebug() << "water_day aggregation (1km): rows=" << rowsWritten
                 << "computation=" << DebugTimer::timeStr(computeMs)
                 << "output=" << DebugTimer::timeStr(outputMs)
                 << "total=" << DebugTimer::timeStr(computeMs + outputMs);
        return;
    }

    foreach(ResourceUnit *ru, m->ruList()) {
        if (ru->id()==-1)
            continue; // do not include if out of project area

        const WaterCycle *wc = ru->waterCycle();
        const ClimateDay *day = ru->climate()->begin();
        const int n_days = ru->climate()->daysOfYear();

        for (int doy = 0; doy < n_days; ++doy, ++day) {
            phaseTimer.start();
            const int dayOfYear = doy + 1;
            const double xCenter = ru->boundingBox().center().x();
            const double yCenter = ru->boundingBox().center().y();
            const double stockedArea = ru->stockedArea()/cRUArea;
            const double stockableArea = ru->stockableArea()/cRUArea;
            const double precipitation = day->preciptitation;
            const double temperature = day->temperature;
            const double et = wc->dailyEvapotranspiration(doy);
            const double laiTotal = wc->dailyEffectiveLAI(doy);
            const double laiBroadleaf = wc->dailyBroadleafLAI(doy);
            const double laiConiferous = wc->dailyConiferousLAI(doy);
            const double swc = wc->dailyWaterContent(doy);
            const double pwp = wc->dailyPWP(doy);
            computeMs += phaseTimer.elapsed() * 1000.;

            phaseTimer.start();
            *this << ru->index()
                  << ru->id()
                  << currentYear()
                  << day->month
                  << dayOfYear             // 1-based day of year
                  << xCenter
                  << yCenter
                  << stockedArea
                  << stockableArea
                  << precipitation
                  << temperature
                  << et
                  << laiTotal
                  << laiBroadleaf
                  << laiConiferous
                  << swc
                  << pwp;
            writeRow();
            outputMs += phaseTimer.elapsed() * 1000.;
            ++rowsWritten;
        }
    }

    qDebug() << "water_day default (100m): rows=" << rowsWritten
             << "computation=" << DebugTimer::timeStr(computeMs)
             << "output=" << DebugTimer::timeStr(outputMs)
             << "total=" << DebugTimer::timeStr(computeMs + outputMs);
}

void DailyWaterOut::setup()
{
    // use a condition for to control execuation for the current year
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);
}

/********************************************************************************************
**    iLand - an individual based forest landscape and disturbance model
**    https://iland-model.org
**    Copyright (C) 2009-  Werner Rammer, Rupert Seidl
**
**    This program is free software: you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation, either version 3 of the License, or
**    (at your option) any later version.
********************************************************************************************/

#include "productionout_daily.h"
#include "debugtimer.h"
#include "model.h"
#include "resourceunit.h"
#include "resourceunitspecies.h"
#include "species.h"
#include "speciesresponse.h"
#include "production3pg.h"
#include "climate.h"

DailyProductionOut::DailyProductionOut()
{
    setName("Daily production per species and resource unit", "production_day");
    setDescription("Daily GPP and environmental response values for each species "
                   "and resource unit. Values represent GPP per m2 of ground area "
                   "before tree-level allocation (effective area, aging).\n"
                   "Note: light competition (LRI) is resolved annually; "
                   "see 'production_month' output for monthly aggregates.\n"
                   "Activate via: <output name='production_day' enabled='true'/>");

    columns() << OutputColumn::year()
              << OutputColumn::ru()
              << OutputColumn::id()
              << OutputColumn::species()
              << OutputColumn("day",    "day of year (1..365/366)",  OutInteger)
              << OutputColumn("month",  "month of year (1..12)",     OutInteger)
              << OutputColumn("tempResponse",
                              "daily temperature response value (0..1); "
                              "3-PG beta function of delayed mean temperature",
                              OutDouble)
              << OutputColumn("waterResponse",
                              "daily soil water response value (0..1); "
                              "linear function of soil water potential psi",
                              OutDouble)
              << OutputColumn("vpdResponse",
                              "daily vapour pressure deficit response (0..1); "
                              "exponential function of VPD [kPa]",
                              OutDouble)
              << OutputColumn("minResponse",
                              "daily minimum of (tempResponse, waterResponse, vpdResponse); "
                              "the actual limiting environmental factor (0..1). "
                              "Zero outside the phenological growing season.",
                              OutDouble)
              << OutputColumn("phenology",
                              "1 if the day is within the phenological growing season "
                              "(leaves are present), 0 otherwise",
                              OutInteger)
              << OutputColumn("radiation_m2",
                              "daily incoming photosynthetically active radiation (PAR) "
                              "in MJ per m2",
                              OutDouble)
              << OutputColumn("utilizableRadiation_m2",
                              "daily utilizable PAR in MJ per m2 "
                              "(= radiation * minResponse; zero outside phenology). "
                              "Sum over year equals monthly utilizableRadiation_m2.",
                              OutDouble)
              << OutputColumn("GPP_kg_m2",
                              "daily gross primary production in kg dry biomass per m2 "
                              "(= utilizableRadiation * epsilon_month, before aging "
                              "and individual tree allocation). "
                              "epsilon includes f_nitrogen and f_CO2 modifiers.",
                              OutDouble);
}

void DailyProductionOut::setup()
{
    // optional condition expression to limit output to specific years
    // e.g. condition = "year % 5 = 0"  to write every 5th year only
    QString condition = settings().value(".condition", "");
    mCondition.setExpression(condition);
}

void DailyProductionOut::execute(const ResourceUnitSpecies *rus)
{
    const Production3PG    &prod = rus->prod3PG();
    //const SpeciesResponse  *resp = prod.mResponse;   // friend access, same as ProductionOut

    // skip species / resource units that had no production this year
    // if (prod.mEnvYear == 0.)
    //     return;

    const Climate    *climate  = rus->ru()->climate();
    const ClimateDay *day      = climate->begin();
    const int         n_days   = climate->daysOfYear();

    // daily arrays added to SpeciesResponse (see speciesresponse.h/.cpp changes below)
    // const double *daily_temp_resp  = resp->dailyTempResponse();
    // const double *daily_water_resp = resp->dailyWaterResponse();
    // const double *daily_vpd_resp   = resp->dailyVpdResponse();
    // const double *daily_min_resp   = resp->dailyMinResponse();
    // const double *daily_urad       = resp->dailyUtilizableRad();
    // const bool   *daily_pheno      = resp->dailyPhenologyActive();

    // daily GPP array added to Production3PG (see production3pg.h/.cpp changes below)
    const double *daily_gpp = prod.dailyGPP();

    for (int doy = 0; doy < n_days; ++doy, ++day) {
        *this << currentYear()
              << rus->ru()->index()
              << rus->ru()->id()
              << rus->species()->id()
              << (doy + 1)             // 1-based day of year
              << day->month
            //   << daily_temp_resp[doy]
            //   << daily_water_resp[doy]
            //   << daily_vpd_resp[doy]
            //   << daily_min_resp[doy]
            //   << (daily_pheno[doy] ? 1 : 0)
              << day->radiation        // raw daily PAR directly from climate
            //  << daily_urad[doy]
              << daily_gpp[doy];
        writeRow();
    }
}

void DailyProductionOut::exec()
{
    DebugTimer t("DailyProductionOut");
    Model *m = GlobalSettings::instance()->model();

    if (!mCondition.isEmpty() && !mCondition.calculate(GlobalSettings::instance()->currentYear()))
        return;

    foreach (ResourceUnit *ru, m->ruList()) {
        if (ru->id() == -1)
            continue; // skip cells outside the project area

        foreach (const ResourceUnitSpecies *rus, ru->ruSpecies()) {
            execute(rus);
        }
    }
}
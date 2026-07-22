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
**    along with this progr5am.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************************************/

#ifndef PRODUCTION3PG_H
#define PRODUCTION3PG_H

class SpeciesResponse;
class ResourceUnit;
class ProductionOut;
class DailyProductionOut;
class Production3PG
{
public:
    Production3PG();
    void setResponse(const SpeciesResponse *response, const ResourceUnit *ru) { mResponse=response; mRu=ru;}
    double calculate(); ///< return  year GPP/rad: kg Biomass/MJ PAR/m2
    void clear(); ///< clear production values
    double rootFraction() const { return mRootFraction; } /// fraction of biomass that should be distributed to roots
    double GPPperArea() const { return mGPPperArea; } ///<  GPP production (yearly) (kg Biomass) per m2 (effective area)
    const double *dailyGPP() const { return mDailyGPPperArea; } ///< daily GPP (kg Biomass) per m2 (effective area) TODO: daily_timestep_check
    double fEnvYear() const { return mEnvYear; } ///< f_env,yr: aggregate environmental factor [0..1}
private:
    inline double calculateUtilizablePAR(const int month) const;
    inline double calculateEpsilon(const int month) const;
    inline double abovegroundFraction() const; ///< calculate fraction of biomass
    const SpeciesResponse *mResponse; ///< species specific responses
    const ResourceUnit *mRu;
    double mUPAR[12]; ///< utilizable radiation MJ/m2 and month
    double mGPP[12]; ///< monthly Gross Primary Production [kg Biomass / m2]
    double mRootFraction; ///< fraction of production that flows into roots
    double mGPPperArea; ///< kg GPP Biomass / m2 interception area
    double mDailyGPPperArea[366]; ///< kg GPP Biomass / m2 interception area per day // TODO: daily_timestep_check
    double mEnvYear; ///< f_env,yr: factor that aggregates the environment for the species over the year (weighted with the radiation pattern)

    friend class ProductionOut;
    friend class DailyProductionOut;
};

#endif // PRODUCTION3PG_H

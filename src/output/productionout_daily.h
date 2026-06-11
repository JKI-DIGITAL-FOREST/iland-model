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

#ifndef DAILYPRODUCTIONOUT_H
#define DAILYPRODUCTIONOUT_H

#include "output.h"
#include "expression.h"

class ResourceUnitSpecies;

/** DailyProductionOut saves daily GPP and environmental response values
 *  per species and resource unit. It is the daily counterpart of ProductionOut.
 *  @ingroup output
 *  @sa https://iland-model.org/primary+production */
class DailyProductionOut : public Output
{
public:
    DailyProductionOut();
    void setup() override;
    void exec() override;
private:
    void execute(const ResourceUnitSpecies *rus);
    Expression mCondition;
};

#endif // DAILYPRODUCTIONOUT_H
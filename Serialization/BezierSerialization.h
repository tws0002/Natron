/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <http://www.natron.fr/>,
 * Copyright (C) 2016 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */

#ifndef Engine_BezierSerialization_h
#define Engine_BezierSerialization_h

#include "Serialization/RotoDrawableItemSerialization.h"
#include "Serialization/BezierCPSerialization.h"
#include "Serialization/SerializationFwd.h"

SERIALIZATION_NAMESPACE_ENTER;

class BezierSerialization
    : public RotoDrawableItemSerialization
{

public:

    BezierSerialization()
        : RotoDrawableItemSerialization()
        , _controlPoints()
        , _closed(false)
        , _isOpenBezier(false)
    {
    }

    virtual ~BezierSerialization()
    {
    }

    virtual void encode(YAML::Emitter& em) const OVERRIDE;

    virtual void decode(const YAML::Node& node) OVERRIDE;
    

    template<class Archive>
    void serialize(Archive & ar,
                   const unsigned int version);

    struct ControlPoint
    {
        BezierCPSerialization innerPoint;

        // Serialize feather only if different
        boost::shared_ptr<BezierCPSerialization> featherPoint;
    };
    std::list< ControlPoint > _controlPoints;


    bool _closed;
    bool _isOpenBezier;
};


SERIALIZATION_NAMESPACE_EXIT;

#endif // Engine_BezierSerialization_h

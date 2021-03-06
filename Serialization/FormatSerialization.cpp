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

#include "FormatSerialization.h"

GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_OFF
#include <yaml-cpp/yaml.h>
GCC_DIAG_UNUSED_LOCAL_TYPEDEFS_ON

SERIALIZATION_NAMESPACE_ENTER

void
FormatSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::Flow;
    em << YAML::BeginSeq;
    em << x1 << y1 << x2 << y2;
    if (par != 1.) {
        em << par;
    }
    if (!name.empty()) {
        em << name;
    }
    em << YAML::EndSeq;

}


void
FormatSerialization::decode(const YAML::Node& node)
{
    if (!node.IsSequence() || node.size() < 4) {
        throw YAML::InvalidNode();
    }
    x1 = node[0].as<int>();
    y1 = node[1].as<int>();
    x2 = node[2].as<int>();
    y2 = node[3].as<int>();
    if (node.size() > 4) {
        try {
            // Try to read par first, this will throw an exception if failed
            par = node[4].as<double>();
            if (node.size() == 6) {
                name = node[5].as<std::string>();
            }
        } catch (const YAML::BadConversion& /*e*/) {
            // No PAR, assume it is 1
            name = node[4].as<std::string>();
        }

    }

}

SERIALIZATION_NAMESPACE_EXIT;

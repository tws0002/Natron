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


#include <cfloat>
#include <climits>
#include <iostream>

#include <yaml-cpp/yaml.h>

#include "KnobSerialization.h"

#include "Serialization/CurveSerialization.h"

SERIALIZATION_NAMESPACE_ENTER;

const std::string&
ValueSerialization::getKnobName() const
{
    return _serialization->getName();
}

void
ValueSerialization::setEnabledChanged(bool b)
{
    KnobSerialization* isKnob = dynamic_cast<KnobSerialization*>(_serialization);
    if (isKnob) {
        isKnob->_disabled = !b;
    }
}

void
KnobSerialization::encode(YAML::Emitter& em) const
{
    if (!_mustSerialize) {
        return;
    }
    em << YAML::BeginMap;
    em << YAML::Key << "ScriptName" << YAML::Value << _scriptName;

    // Instead of inserting in the map each boolean
    // we serialize strings, meaning that
    // if they are here, their value is true otherwise it goes to default
    std::list<std::string> propNames;

    if (_masterIsAlias) {
        propNames.push_back("MasterIsAlias");
    }
    assert((int)_values.size() == _dimension);


    int nDimsToSerialize = 0;
    int nDimsWithValue = 0;
    int nDimsWithDefValue = 0;
    for (std::size_t i = 0; i < _values.size(); ++i) {
        if (_values[i]._mustSerialize && _isPersistent) {
            ++nDimsToSerialize;
            if (_values[i]._serializeValue || !_values[i]._animationCurve.keys.empty() || _values[i]._slaveMasterLink.hasLink || !_values[i]._expression.empty()) {
                ++nDimsWithValue;
            }
            if (_values[i]._serializeDefaultValue) {
                ++nDimsWithDefValue;
            }
        }
    }


    if (nDimsToSerialize > 0) {
        // We serialize a value, we need to know which type of knob this is because at the time of deserialization the
        // application may not have created the corresponding knob already and may not know to what type it corresponds
        if (nDimsWithValue > 0) {
            em << YAML::Key << "Value" << YAML::Value;
            // Starting dimensions
            if (_values.size() > 1) {
                em << YAML::Flow << YAML::BeginSeq;
            }
            for (std::size_t i = 0; i < _values.size(); ++i) {
                if (_values[i]._slaveMasterLink.hasLink) {
                    // Wrap the link in a sequence of 1 element to distinguish with regular string knobs values
                    em << YAML::Flow << YAML::BeginMap;
                    std::stringstream ss;
                    // Encode the hard-link into a single string
                    if (!_values[i]._slaveMasterLink.masterNodeName.empty()) {
                        em << YAML::Key << "N";
                        em << YAML::Value << _values[i]._slaveMasterLink.masterNodeName;
                    }
                    if (!_values[i]._slaveMasterLink.masterTrackName.empty()) {
                        em << YAML::Key << "T";
                        em << YAML::Value << _values[i]._slaveMasterLink.masterTrackName;
                    }
                    if (!_values[i]._slaveMasterLink.masterKnobName.empty()) {
                        em << YAML::Key << "K";
                        em << YAML::Value << _values[i]._slaveMasterLink.masterKnobName;
                    }
                    if (!_values[i]._slaveMasterLink.masterDimensionName.empty()) {
                        em << YAML::Key << "D";
                        em << YAML::Value << _values[i]._slaveMasterLink.masterDimensionName;
                    }
                    em << YAML::EndMap;
                } else if (!_values[i]._expression.empty()) {
                    // Wrap the expression in a sequence of 1 element to distinguish with regular string knobs values
                    em << YAML::Flow << YAML::BeginMap;
                    if (_values[i]._expresionHasReturnVariable) {
                        // Multi-line expr
                        em << YAML::Key << "MultiExpr";
                    } else {
                        // single line expr
                        em << YAML::Key << "Expr";
                    }
                    em << YAML::Value << _values[i]._expression;
                    em << YAML::EndMap;
                } else if (!_values[i]._animationCurve.keys.empty()) {
                    em << YAML::Flow << YAML::BeginMap;
                    em << YAML::Key << "Curve" << YAML::Value;
                    _values[i]._animationCurve.encode(em);
                    em << YAML::EndMap;
                } else {
                    assert(_values[i]._type != ValueSerialization::eSerializationValueVariantTypeNone);
                    switch (_values[i]._type) {
                        case ValueSerialization::eSerializationValueVariantTypeBoolean:
                            em << _values[i]._value.isBool;
                            break;
                        case ValueSerialization::eSerializationValueVariantTypeInteger:
                            em << _values[i]._value.isInt;
                            break;
                        case ValueSerialization::eSerializationValueVariantTypeDouble:
                            em << _values[i]._value.isDouble;
                            break;
                        case ValueSerialization::eSerializationValueVariantTypeString:
                            em << _values[i]._value.isString;
                            break;
                        default:
                            break;
                    }
                }
            }
            if (_values.size() > 1) {
                em << YAML::EndSeq;
            }
        }


        if (nDimsWithDefValue) {
            em << YAML::Key << "Default" << YAML::Value;
            // Starting dimensions
            if (_values.size() > 1) {
                em << YAML::Flow << YAML::BeginSeq;
            }
            for (std::size_t i = 0; i < _values.size(); ++i) {
                switch (_values[i]._type) {
                    case ValueSerialization::eSerializationValueVariantTypeBoolean:
                        em << _values[i]._defaultValue.isBool;
                        break;
                    case ValueSerialization::eSerializationValueVariantTypeInteger:
                        em << _values[i]._defaultValue.isInt;
                        break;
                    case ValueSerialization::eSerializationValueVariantTypeDouble:
                        em << _values[i]._defaultValue.isDouble;
                        break;
                    case ValueSerialization::eSerializationValueVariantTypeString:
                        em << _values[i]._defaultValue.isString;
                        break;
                    default:
                        break;
                }

            }
            if (_values.size() > 1) {
                em << YAML::EndSeq;
            }
        }

    } // hasDimensionToSerialize

    TextExtraData* textData = dynamic_cast<TextExtraData*>(_extraData.get());
    if (_extraData && _isPersistent) {
        ParametricExtraData* parametricData = dynamic_cast<ParametricExtraData*>(_extraData.get());
        if (parametricData) {
            if (!parametricData->parametricCurves.empty()) {
                em << YAML::Key << "ParametricCurves" << YAML::Value;
                em << YAML::BeginSeq;
                for (std::list<CurveSerialization>::const_iterator it = parametricData->parametricCurves.begin(); it!=parametricData->parametricCurves.end(); ++it) {
                    it->encode(em);
                }
                em << YAML::EndSeq;
            }
        } else if (textData) {
            if (!textData->keyframes.empty()) {
                em << YAML::Key << "TextAnim" << YAML::Value;
                em << YAML::Flow;
                em << YAML::BeginSeq;
                for (std::map<int, std::string>::const_iterator it = textData->keyframes.begin(); it!=textData->keyframes.end(); ++it) {
                    em << it->first << it->second;
                }
                em << YAML::EndSeq;
            }
            if (std::abs(textData->fontColor[0] - 0.) > 0.01 || std::abs(textData->fontColor[1] - 0.) > 0.01 || std::abs(textData->fontColor[2] - 0.) > 0.01) {
                em << YAML::Key << "FontColor" << YAML::Value << YAML::Flow << YAML::BeginSeq << textData->fontColor[0] << textData->fontColor[1] << textData->fontColor[2] << YAML::EndSeq;
            }
            if (textData->fontSize != kKnobStringDefaultFontSize) {
                em << YAML::Key << "FontSize" << YAML::Value << textData->fontSize;
            }
            if (textData->fontFamily != NATRON_FONT) {
                em << YAML::Key << "Font" << YAML::Value << textData->fontFamily;
            }
            if (textData->italicActivated) {
                propNames.push_back("Italic");
            }
            if (textData->boldActivated) {
                propNames.push_back("Bold");
            }
        }
    }

    if (_isUserKnob) {
        // Num of dimensions can be figured out for knobs created by plug-in
        // Knobs created by user need to know it
        em << YAML::Key << "NDims" << YAML::Value << _dimension;
        em << YAML::Key << "TypeName" << YAML::Value << _typeName;

        if (_label != _scriptName) {
            em << YAML::Key << "Label" << YAML::Value << _label;
        }
        if (!_tooltip.empty()) {
            em << YAML::Key << "Hint" << YAML::Value << _tooltip;
        }
        if (_isSecret) {
            propNames.push_back("Secret");
        }

        if (_disabled) {
            propNames.push_back("Disabled");
        }

        if (!_triggerNewLine) {
            propNames.push_back("NoNewLine");
        }
        if (!_evaluatesOnChange) {
            propNames.push_back("NoEval");
        }
        if (_animatesChanged) {
            propNames.push_back("AnimatesChanged");
        }
        if (!_isPersistent) {
            propNames.push_back("Volatile");
        }
        if (!_iconFilePath[0].empty()) {
            em << YAML::Key << "UncheckedIcon" << YAML::Value << _iconFilePath[0];
        }
        if (!_iconFilePath[1].empty()) {
            em << YAML::Key << "CheckedIcon" << YAML::Value << _iconFilePath[1];
        }

        TypeExtraData* typeData = _extraData.get();
        ChoiceExtraData* cdata = dynamic_cast<ChoiceExtraData*>(_extraData.get());
        ValueExtraData* vdata = dynamic_cast<ValueExtraData*>(_extraData.get());
        TextExtraData* tdata = dynamic_cast<TextExtraData*>(_extraData.get());
        FileExtraData* fdata = dynamic_cast<FileExtraData*>(_extraData.get());
        PathExtraData* pdata = dynamic_cast<PathExtraData*>(_extraData.get());

        if (cdata) {
            if (!cdata->_entries.empty()) {
                em << YAML::Key << "Entries" << YAML::Value;
                em << YAML::Flow << YAML::BeginSeq;
                for (std::size_t i = 0; i < cdata->_entries.size(); ++i) {
                    em << cdata->_entries[i];
                }
                em << YAML::EndSeq;
            }
            if (!cdata->_helpStrings.empty()) {
                bool hasHelp = false;
                for (std::size_t i = 0; i < cdata->_helpStrings.size(); ++i) {
                    if (!cdata->_helpStrings[i].empty()) {
                        hasHelp = true;
                        break;
                    }
                }
                if (hasHelp) {
                    em << YAML::Key << "Hints" << YAML::Value;
                    em <<  YAML::Flow << YAML::BeginSeq;
                    for (std::size_t i = 0; i < cdata->_helpStrings.size(); ++i) {
                        em << cdata->_helpStrings[i];
                    }
                    em << YAML::EndSeq;
                }
            }
        } else if (vdata) {
            if (vdata->min != INT_MIN && vdata->min != -DBL_MAX) {
                em << YAML::Key << "Min" << YAML::Value << vdata->min;
            }
            if (vdata->min != INT_MAX && vdata->min != DBL_MAX) {
                em << YAML::Key << "Max" << YAML::Value << vdata->max;
            }
            if (vdata->min != INT_MIN && vdata->min != -DBL_MAX) {
                em << YAML::Key << "DisplayMin" << YAML::Value << vdata->dmin;
            }
            if (vdata->min != INT_MAX && vdata->min != DBL_MAX) {
                em << YAML::Key << "DisplayMax" << YAML::Value << vdata->dmax;
            }
        } else if (tdata) {
            if (tdata->label) {
                propNames.push_back("IsLabel");
            }
            if (tdata->multiLine) {
                propNames.push_back("MultiLine");
            }
            if (tdata->richText) {
                propNames.push_back("RichText");
            }
        } else if (pdata) {
            if (pdata->multiPath) {
                propNames.push_back("MultiPath");
            }
        } else if (fdata) {
            if (fdata->useSequences) {
                propNames.push_back("Sequences");
            }
            if (fdata->useExistingFiles) {
                propNames.push_back("ExistingFiles");
            }
            if (!fdata->filters.empty()) {
                em << YAML::Key << "FileTypes" << YAML::Value << YAML::Flow << YAML::BeginSeq;
                for (std::size_t i = 0; i < fdata->filters.size(); ++i) {
                    em << fdata->filters[i];
                }
                em << YAML::EndSeq;
            }
        }

        if (typeData) {
            if (typeData->useHostOverlayHandle) {
                propNames.push_back("UseOverlay");
            }
        }

    } // if (_isUserKnob) {

    if (_hasViewerInterface) {
        if (!_inViewerContextItemLayout.empty() && _inViewerContextItemLayout != kInViewerContextItemLayoutSpacing) {
            em << YAML::Key << "InViewerLayout" << YAML::Value << _inViewerContextItemLayout;
        } else {
            if (_inViewerContextItemSpacing != kKnobInViewerContextDefaultItemSpacing) {
                em << YAML::Key << "InViewerSpacing" << YAML::Value << _inViewerContextItemSpacing;
            }
        }
        if (_isUserKnob) {
            if (!_inViewerContextLabel.empty()) {
                em << YAML::Key << "InViewerLabel" << YAML::Value << _inViewerContextLabel;
            }
            if (!_inViewerContextIconFilePath[0].empty()) {
                em << YAML::Key << "InViewerIconUnchecked" << YAML::Value << _inViewerContextIconFilePath[0];
            }
            if (!_inViewerContextIconFilePath[1].empty()) {
                em << YAML::Key << "InViewerIconChecked" << YAML::Value << _inViewerContextIconFilePath[1];
            }
        }
    }

    if (!propNames.empty()) {
        em << YAML::Key << "Props" << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (std::list<std::string>::const_iterator it2 = propNames.begin(); it2 != propNames.end(); ++it2) {
            em << *it2;
        }
        em << YAML::EndSeq;
    }

    em << YAML::EndMap;
} // KnobSerialization::encode

template <typename T>
static T* getOrCreateExtraData(boost::scoped_ptr<TypeExtraData>& extraData)
{
    T* data = 0;
    if (extraData.get()) {
        data = dynamic_cast<T*>(extraData.get());
    } else {
        data = new T;
        extraData.reset(data);
    }
    assert(data);
    if (!data) {
        throw YAML::InvalidNode();
    }
    return data;
}

static void decodeValueFromNode(const YAML::Node& node,
                                SerializationValueVariant& variant, ValueSerialization::SerializationValueVariantTypeEnum* type)
{
    // yaml-cpp looses the original type information and does not seem to make a difference whether
    // the value was a string or a POD scalar. All functions as<T>() will succeed.
    // See https://github.com/jbeder/yaml-cpp/issues/261

    try {
        *type = ValueSerialization::eSerializationValueVariantTypeDouble;
        variant.isDouble = node.as<double>();
        variant.isInt = (int)variant.isDouble; //node.as<int>();
    } catch (const YAML::BadConversion&) {
        try {
            variant.isBool = node.as<bool>();
            *type = ValueSerialization::eSerializationValueVariantTypeBoolean;
        } catch (const YAML::BadConversion&) {
            variant.isString = node.as<std::string>();
            *type = ValueSerialization::eSerializationValueVariantTypeString;

        }
    }
    
}

static void initValuesVec(KnobSerialization* serialization, int nDims)
{
    if ((int)serialization->_values.size() != nDims) {
        serialization->_dimension = nDims;
        serialization->_values.resize(nDims);
        for (std::size_t i = 0; i < serialization->_values.size(); ++i) {
            serialization->_values[i]._serialization = serialization;
            serialization->_values[i]._mustSerialize = true;
            serialization->_values[i]._dimension = i;
        }
    }
}


void
KnobSerialization::decode(const YAML::Node& node)
{
    if (!node.IsMap()) {
        return;
    }

    // Set the flag to true if the user use this object directly to encode after
    _mustSerialize = true;

    _scriptName = node["ScriptName"].as<std::string>();

    if (node["Value"]) {
        YAML::Node valueNode = node["Value"];

        int nDims = valueNode.IsSequence() ? valueNode.size() : 1;
        initValuesVec(this, nDims);

        for (int i = 0; i < nDims; ++i) {

            YAML::Node dimNode = valueNode.IsSequence() ? valueNode[i] : valueNode;

            if (!dimNode.IsMap()) {
                // This is a value
                decodeValueFromNode(dimNode, _values[i]._value, &_values[i]._type);
                _values[i]._serializeValue = true;
            } else { // if (dimNode[i].isMap())
                if (dimNode["Curve"]) {
                    // Curve
                    _values[i]._animationCurve.decode(dimNode["Curve"]);
                } else if (dimNode["MultiExpr"]) {
                    _values[i]._expression = dimNode["MultiExpr"].as<std::string>();
                    _values[i]._expresionHasReturnVariable = true;
                } else if (dimNode["Expr"]) {
                    _values[i]._expression = dimNode["Expr"].as<std::string>();
                } else {
                    // This is most likely a regular slavr/master link
                    if (dimNode["N"]) {
                        _values[i]._slaveMasterLink.masterNodeName = dimNode["N"].as<std::string>();
                    }
                    if (dimNode["T"]) {
                        _values[i]._slaveMasterLink.masterTrackName = dimNode["T"].as<std::string>();
                    }
                    if (dimNode["K"]) {
                        _values[i]._slaveMasterLink.masterKnobName = dimNode["K"].as<std::string>();
                    }
                    if (dimNode["D"]) {
                        _values[i]._slaveMasterLink.masterDimensionName = dimNode["D"].as<std::string>();
                    }

                }

            } // if (!valueNode[i].IsSequence())
        }
    }

    if (node["Default"]) {

        YAML::Node defNode = node["Default"];
        int nDims = defNode.IsSequence() ? defNode.size() : 1;
        initValuesVec(this, nDims);
        for (int i = 0; i < nDims; ++i) {
            _values[i]._serializeDefaultValue = true;
            YAML::Node dimNode = defNode.IsSequence() ? defNode[i] : defNode;
            decodeValueFromNode(dimNode, _values[i]._defaultValue, &_values[i]._type);

        }
    }

    if (node["ParametricCurves"]) {
        YAML::Node curveNode = node["ParametricCurves"];
        ParametricExtraData *data = getOrCreateExtraData<ParametricExtraData>(_extraData);
        for (std::size_t i = 0; i < curveNode.size(); ++i) {
            CurveSerialization s;
            s.decode(curveNode[i]);
            data->parametricCurves.push_back(s);
        }
    }
    if (node["TextAnim"]) {
        YAML::Node curveNode = node["TextAnim"];
        TextExtraData *data = getOrCreateExtraData<TextExtraData>(_extraData);
        // If type = 0 we expect a int, otherwise a string
        int type = 0;
        std::pair<int, std::string> p;
        for (std::size_t i = 0; i < curveNode.size(); ++i) {
            if (type == 0) {
                p.first = curveNode[i].as<int>();
                type = 1;
            } else if (type == 1) {
                type = 0;
                p.second = curveNode[i].as<std::string>();
                data->keyframes.insert(p);
            }
        }
    }

    if (node["FontColor"]) {
        YAML::Node n = node["FontColor"];
        if (n.size() != 3) {
            throw YAML::InvalidNode();
        }
        TextExtraData *data = getOrCreateExtraData<TextExtraData>(_extraData);
        data->fontColor[0] = n[0].as<double>();
        data->fontColor[1] = n[1].as<double>();
        data->fontColor[2] = n[2].as<double>();

    }
    if (node["FontSize"]) {
        TextExtraData *data = getOrCreateExtraData<TextExtraData>(_extraData);
        data->fontSize = node["FontSize"].as<int>();
    }

    if (node["Font"]) {
        TextExtraData *data = getOrCreateExtraData<TextExtraData>(_extraData);
        data->fontFamily = node["Font"].as<std::string>();
    }

    if (node["NDims"]) {

        // This is a user knob
        _isUserKnob = true;

        _typeName = node["TypeName"].as<std::string>();

        _dimension = node["NDims"].as<int>();

        if (node["Label"]) {
            _label = node["Label"].as<std::string>();
        } else {
            _label = _scriptName;
        }
        if (node["Hint"]) {
            _tooltip = node["Hint"].as<std::string>();
        }

        if (node["Persistent"]) {
            _isPersistent = node["Persistent"].as<bool>();
        } else {
            _isPersistent = true;
        }
        if (node["UncheckedIcon"]) {
            _iconFilePath[0] = node["UncheckedIcon"].as<std::string>();
        }
        if (node["CheckedIcon"]) {
            _iconFilePath[1] = node["CheckedIcon"].as<std::string>();
        }

        // User specific data
        if (node["Entries"]) {
            // This is a choice
            ChoiceExtraData *data = new ChoiceExtraData;
            _extraData.reset(data);
            YAML::Node entriesNode = node["Entries"];
            for (std::size_t i = 0; i < entriesNode.size(); ++i) {
                data->_entries.push_back(entriesNode[i].as<std::string>());
            }

            // Also look for hints...
            if (node["Hints"]) {
                YAML::Node hintsNode = node["Hints"];
                for (std::size_t i = 0; i < hintsNode.size(); ++i) {
                    data->_helpStrings.push_back(hintsNode[i].as<std::string>());
                }
            }
        }

        if (node["Min"]) {
            ValueExtraData* data = getOrCreateExtraData<ValueExtraData>(_extraData);
            data->min = node["Min"].as<double>();
        }
        if (node["Max"]) {
            ValueExtraData* data = getOrCreateExtraData<ValueExtraData>(_extraData);
            data->max = node["Max"].as<double>();
        }
        if (node["DisplayMin"]) {
            ValueExtraData* data = getOrCreateExtraData<ValueExtraData>(_extraData);
            data->dmin = node["DisplayMin"].as<double>();
        }
        if (node["DisplayMax"]) {
            ValueExtraData* data = getOrCreateExtraData<ValueExtraData>(_extraData);
            data->dmax = node["DisplayMax"].as<double>();
        }
        if (node["FileTypes"]) {
            FileExtraData* data = getOrCreateExtraData<FileExtraData>(_extraData);
            YAML::Node fileTypesNode = node["FileTypes"];
            for (std::size_t i = 0; i < fileTypesNode.size(); ++i) {
                data->filters.push_back(fileTypesNode[i].as<std::string>());
            }
        }
    } // isUserKnob

    if (node["InViewerLayout"]) {
        _inViewerContextItemLayout = node["InViewerLayout"].as<std::string>();
        _hasViewerInterface = true;
    }

    if (node["InViewerSpacing"]) {
        _inViewerContextItemSpacing = node["InViewerSpacing"].as<int>();
        _hasViewerInterface = true;
    }

    if (_isUserKnob) {
        if (node["InViewerLabel"]) {
            _inViewerContextLabel = node["InViewerLabel"].as<std::string>();
            _hasViewerInterface = true;
        }
        if (node["InViewerIconUnchecked"]) {
            _inViewerContextIconFilePath[0] = node["InViewerIconUnchecked"].as<std::string>();
            _hasViewerInterface = true;
        }
        if (node["InViewerIconChecked"]) {
            _inViewerContextIconFilePath[1] = node["InViewerIconChecked"].as<std::string>();
            _hasViewerInterface = true;
        }
    }

    if (node["Props"]) {
        YAML::Node propsNode = node["Props"];
        for (std::size_t i = 0; i < propsNode.size(); ++i) {
            std::string prop = propsNode[i].as<std::string>();
            if (prop == "MasterIsAlias") {
                _masterIsAlias = true;
            } else if (prop == "Secret") {
                _isSecret = true;
            } else if (prop == "Disabled") {
                _disabled = true;
            } else if (prop == "NoNewLine") {
                _triggerNewLine = false;
            } else if (prop == "NoEval") {
                _evaluatesOnChange = false;
            } else if (prop == "AnimatesChanged") {
                _animatesChanged = true;
            } else if (prop == "Volatile") {
                _isPersistent = false;
            } else if (prop == "IsLabel") {
                TextExtraData* data = getOrCreateExtraData<TextExtraData>(_extraData);
                data->label = true;
                data->multiLine = false;
                data->richText = false;
            } else if (prop == "MultiLine") {
                TextExtraData* data = getOrCreateExtraData<TextExtraData>(_extraData);
                data->label = false;
                data->multiLine = true;
                data->richText = false;
            } else if (prop == "RichText") {
                TextExtraData* data = getOrCreateExtraData<TextExtraData>(_extraData);
                data->richText = true;
            } else if (prop == "MultiPath") {
                PathExtraData* data = getOrCreateExtraData<PathExtraData>(_extraData);
                data->multiPath = node["MultiPath"].as<bool>();
            } else if (prop == "Sequences") {
                FileExtraData* data = getOrCreateExtraData<FileExtraData>(_extraData);
                data->useSequences = node["Sequences"].as<bool>();
            } else if (prop == "ExistingFiles") {
                FileExtraData* data = getOrCreateExtraData<FileExtraData>(_extraData);
                data->useExistingFiles = node["ExistingFiles"].as<bool>();
            } else if (prop == "UseOverlay") {
                TypeExtraData* data = getOrCreateExtraData<TypeExtraData>(_extraData);
                data->useHostOverlayHandle = true;
            } else if (prop == "Italic") {
                TextExtraData *data = getOrCreateExtraData<TextExtraData>(_extraData);
                data->italicActivated = true;
            } else if (prop == "Bold") {
                TextExtraData *data = getOrCreateExtraData<TextExtraData>(_extraData);
                data->boldActivated = true;
            } else {
                std::cerr << "WARNING: Unrecognized parameter property " << prop << std::endl;
            }


        }

    }
    
} // KnobSerialization::decode


void
GroupKnobSerialization::encode(YAML::Emitter& em) const
{
    em << YAML::BeginMap;

    em << YAML::Key << "TypeName" << YAML::Value << _typeName;
    em << YAML::Key << "ScriptName" << YAML::Value << _name;
    if (_label != _name) {
        em << YAML::Key << "Label" << YAML::Value << _label;
    }

    if (!_children.empty()) {
        em << YAML::Key << "Params" << YAML::Value << YAML::BeginSeq;
        for (std::list <boost::shared_ptr<KnobSerializationBase> >::const_iterator it = _children.begin(); it!=_children.end(); ++it) {
            (*it)->encode(em);
        }
        em << YAML::EndSeq;
    }

    
    
    std::list<std::string> props;
    if (_isOpened) {
        props.push_back("Opened");
    }
    if (_isSetAsTab) {
        props.push_back("IsTab");
    }
    if (_secret) {
        props.push_back("Secret");
    }
    if (!props.empty()) {
        em << YAML::Key << "Props" << YAML::Value << YAML::BeginSeq;
        for (std::list<std::string>::const_iterator it = props.begin(); it!=props.end(); ++it) {
            em << *it;
        }
        em << YAML::EndSeq;
    }
    em << YAML::EndMap;
} // GroupKnobSerialization::encode

void
GroupKnobSerialization::decode(const YAML::Node& node)
{
    _typeName = node["TypeName"].as<std::string>();
    _name = node["ScriptName"].as<std::string>();
    if (node["Label"]) {
        _label = node["Label"].as<std::string>();
    } else {
        _label = _name;
    }

    if (node["Params"]) {
        YAML::Node paramsNode = node["Params"];
        for (std::size_t i = 0; i < paramsNode.size(); ++i) {

            std::string typeName;
            if (paramsNode[i]["TypeName"]) {
                typeName = paramsNode[i]["TypeName"].as<std::string>();
            }

            if (typeName == kKnobPageTypeName || typeName == kKnobGroupTypeName) {
                GroupKnobSerializationPtr s(new GroupKnobSerialization);
                s->decode(paramsNode[i]);
                _children.push_back(s);
            } else {
                KnobSerializationPtr s (new KnobSerialization);
                s->decode(paramsNode[i]);
                _children.push_back(s);
            }
        }
    }

    if (node["Props"]) {
        YAML::Node propsNode = node["Props"];
        for (std::size_t i = 0; i < propsNode.size(); ++i) {
            std::string prop = propsNode[i].as<std::string>();
            if (prop == "Opened") {
                _isOpened = true;
            } else if (prop == "Secret") {
                _secret = true;
            } else if (prop == "IsTab") {
                _isSetAsTab = true;
            }
        }
    }
} // GroupKnobSerialization::decode

SERIALIZATION_NAMESPACE_EXIT;

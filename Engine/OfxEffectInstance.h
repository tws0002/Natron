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

#ifndef NATRON_ENGINE_OFXNODE_H
#define NATRON_ENGINE_OFXNODE_H

// ***** BEGIN PYTHON BLOCK *****
// from <https://docs.python.org/3/c-api/intro.html#include-files>:
// "Since Python may define some pre-processor definitions which affect the standard headers on some systems, you must include Python.h before any standard headers are included."
#include <Python.h>
// ***** END PYTHON BLOCK *****

#include "Global/Macros.h"

#include <map>
#include <string>

#if !defined(Q_MOC_RUN) && !defined(SBK_RUN)
#include <boost/shared_ptr.hpp>
#endif

CLANG_DIAG_OFF(deprecated)
#include <QtCore/QMutex>
#include <QtCore/QString>
#include <QtCore/QObject>
CLANG_DIAG_ON(deprecated)
#include <QtCore/QStringList>
//ofx
CLANG_DIAG_OFF(unknown-pragmas)
CLANG_DIAG_OFF(tautological-undefined-compare)
#include <ofxhImageEffect.h>
CLANG_DIAG_ON(tautological-undefined-compare)
CLANG_DIAG_ON(unknown-pragmas)

#include "Engine/OutputEffectInstance.h"
#include "Engine/ViewIdx.h"

#include "Engine/EngineFwd.h"

NATRON_NAMESPACE_ENTER;

class AbstractOfxEffectInstance
    : public OutputEffectInstance
{
protected: // derives from EffectInstance, parent of OfxEffectInstance
    // TODO: enable_shared_from_this
    // constructors should be privatized in any class that derives from boost::enable_shared_from_this<>
    AbstractOfxEffectInstance(const NodePtr& node)
        : OutputEffectInstance(node)
    {
    }


    AbstractOfxEffectInstance(const AbstractOfxEffectInstance& other)
    : OutputEffectInstance(other)
    {
    }

public:
    //static EffectInstancePtr create(const NodePtr& node) WARN_UNUSED_RETURN
    //{
    //    return EffectInstancePtr( new AbstractOfxEffectInstance(node) );
    //}

    virtual ~AbstractOfxEffectInstance()
    {
    }

    static std::vector<std::string> makePluginGrouping(const std::string & pluginIdentifier,
                                          int versionMajor, int versionMinor,
                                          const std::string & pluginLabel,
                                          const std::string & grouping) WARN_UNUSED_RETURN;
    static std::string makePluginLabel(const std::string & shortLabel,
                                       const std::string & label,
                                       const std::string & longLabel) WARN_UNUSED_RETURN;
};

struct OfxEffectInstancePrivate;
class OfxEffectInstance
    : public AbstractOfxEffectInstance
{
GCC_DIAG_SUGGEST_OVERRIDE_OFF
    Q_OBJECT
GCC_DIAG_SUGGEST_OVERRIDE_ON

private: // derives from EffectInstance
    // TODO: enable_shared_from_this
    // constructors should be privatized in any class that derives from boost::enable_shared_from_this<>
    OfxEffectInstance(const NodePtr& node);

    OfxEffectInstance(const OfxEffectInstance& other);

public:
    static EffectInstancePtr create(const NodePtr& node) WARN_UNUSED_RETURN
    {
        return EffectInstancePtr( new OfxEffectInstance(node) );
    }

    virtual ~OfxEffectInstance();


    OfxImageEffectInstance* effectInstance() WARN_UNUSED_RETURN;
    const OfxImageEffectInstance* effectInstance() const WARN_UNUSED_RETURN;
    const std::string & getShortLabel() const WARN_UNUSED_RETURN;

    typedef std::vector<OFX::Host::ImageEffect::ClipDescriptor*> MappedInputV;
    MappedInputV inputClipsCopyWithoutOutput() const WARN_UNUSED_RETURN;

    bool isInitialized() const;

    const std::string & ofxGetOutputPremultiplication() const;

    /**
     * @brief Calls syncPrivateDataAction from another thread than the main thread. The actual
     * call of the action will take place in the main-thread.
     **/
    void syncPrivateData_other_thread();

public:
    /********OVERRIDEN FROM EFFECT INSTANCE*************/
    virtual void initializeDataAfterCreate() OVERRIDE FINAL;
    virtual void createInstanceAction() OVERRIDE FINAL;
    virtual bool isGenerator() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isReader() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isWriter() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isVideoWriter() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isOutput() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isGeneratorAndFilter() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isOpenFX() const OVERRIDE FINAL WARN_UNUSED_RETURN
    {
        return true;
    }
    virtual PluginOpenGLRenderSupport getCurrentOpenGLSupport() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual RenderSafetyEnum getCurrentRenderThreadSafety() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void onScriptNameChanged(const std::string& fullyQualifiedName) OVERRIDE FINAL;
    virtual bool isEffectCreated() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual int getMaxInputCount() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual std::string getInputLabel (int inputNb) const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual std::string getInputHint(int inputNb) const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isInputOptional(int inputNb) const OVERRIDE WARN_UNUSED_RETURN;
    virtual bool isInputMask(int inputNb) const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual StatusEnum getRegionOfDefinition(double time, const RenderScale & scale, ViewIdx view, RectD* rod) OVERRIDE WARN_UNUSED_RETURN;

    /// calculate the default rod for this effect instance
    virtual void calcDefaultRegionOfDefinition(double time, const RenderScale & scale, ViewIdx view, RectD *rod)  OVERRIDE;
    virtual void getRegionsOfInterest(double time,
                                      const RenderScale & scale,
                                      const RectD & outputRoD, //!< full RoD in canonical coordinates
                                      const RectD & renderWindow, //!< the region to be rendered in the output image, in Canonical Coordinates
                                      ViewIdx view,
                                      RoIMap* ret) OVERRIDE FINAL;
    virtual FramesNeededMap getFramesNeeded(double time, ViewIdx view) OVERRIDE WARN_UNUSED_RETURN;
    virtual void getFrameRange(double *first, double *last) OVERRIDE;
    virtual void initializeOverlayInteract() OVERRIDE FINAL;
    virtual bool hasOverlay() const OVERRIDE FINAL;
    virtual void redrawOverlayInteract() OVERRIDE FINAL;
    virtual void drawOverlay(double time, const RenderScale & renderScale, ViewIdx view) OVERRIDE FINAL;
    virtual bool onOverlayPenDown(double time, const RenderScale & renderScale, ViewIdx view, const QPointF & viewportPos, const QPointF & pos, double pressure, double timestamp, PenType pen) OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool onOverlayPenMotion(double time, const RenderScale & renderScale, ViewIdx view,
                                    const QPointF & viewportPos, const QPointF & pos, double pressure, double timestamp) OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool onOverlayPenUp(double time, const RenderScale & renderScale, ViewIdx view, const QPointF & viewportPos, const QPointF & pos, double pressure, double timestamp) OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool onOverlayKeyDown(double time, const RenderScale & renderScale, ViewIdx view, Key key, KeyboardModifiers modifiers) OVERRIDE FINAL;
    virtual bool onOverlayKeyUp(double time, const RenderScale & renderScale, ViewIdx view, Key key, KeyboardModifiers modifiers) OVERRIDE FINAL;
    virtual bool onOverlayKeyRepeat(double time, const RenderScale & renderScale, ViewIdx view, Key key, KeyboardModifiers modifiers) OVERRIDE FINAL;
    virtual bool onOverlayFocusGained(double time, const RenderScale & renderScale, ViewIdx view) OVERRIDE FINAL;
    virtual bool onOverlayFocusLost(double time, const RenderScale & renderScale, ViewIdx view) OVERRIDE FINAL;
    virtual bool canHandleRenderScaleForOverlays() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void setCurrentViewportForOverlays(OverlaySupport* viewport) OVERRIDE FINAL;
    virtual void beginKnobsValuesChanged(ValueChangedReasonEnum reason) OVERRIDE;
    virtual void endKnobsValuesChanged(ValueChangedReasonEnum reason) OVERRIDE;
    virtual bool knobChanged(const KnobIPtr& k,
                             ValueChangedReasonEnum reason,
                             ViewSpec view,
                             double time,
                             bool originatedFromMainThread) OVERRIDE;
    virtual void beginEditKnobs() OVERRIDE;
    virtual StatusEnum render(const RenderActionArgs& args) OVERRIDE WARN_UNUSED_RETURN;
    virtual bool isIdentity(double time,
                            const RenderScale & scale,
                            const RectI & renderWindow, //!< render window in pixel coords
                            ViewIdx view,
                            double* inputTime,
                            ViewIdx* inputView,
                            int* inputNb) OVERRIDE;
    virtual void purgeCaches() OVERRIDE;

    /**
     * @brief Does this effect supports tiling ?
     * http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#kOfxImageEffectPropSupportsTiles
     * If a clip or plugin does not support tiled images, then the host should supply
     * full RoD images to the effect whenever it fetches one.
     **/
    virtual bool supportsTiles() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool supportsRenderQuality() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool doesTemporalClipAccess() const OVERRIDE FINAL WARN_UNUSED_RETURN;

    /**
     * @brief Does this effect supports multiresolution ?
     * http://openfx.sourceforge.net/Documentation/1.3/ofxProgrammingReference.html#kOfxImageEffectPropSupportsMultiResolution
     * Multiple resolution images mean...
     * input and output images can be of any size
     * input and output images can be offset from the origin
     **/
    virtual bool supportsMultiResolution() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool supportsMultipleClipPARs() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool supportsMultipleClipDepths() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isHostChannelSelectorSupported(bool* defaultR, bool* defaultG, bool* defaultB, bool* defaultA) const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void onInputChanged(int inputNo) OVERRIDE FINAL;
    virtual StatusEnum beginSequenceRender(double first,
                                           double last,
                                           double step,
                                           bool interactive,
                                           const RenderScale & scale,
                                           bool isSequentialRender,
                                           bool isRenderResponseToUserInteraction,
                                           bool draftMode,
                                           ViewIdx view,
                                           bool isOpenGLRender,
                                           const EffectOpenGLContextDataPtr& glContextData) OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual StatusEnum endSequenceRender(double first,
                                         double last,
                                         double step,
                                         bool interactive,
                                         const RenderScale & scale,
                                         bool isSequentialRender,
                                         bool isRenderResponseToUserInteraction,
                                         bool draftMode,
                                         ViewIdx view,
                                         bool isOpenGLRender,
                                         const EffectOpenGLContextDataPtr& glContextData) OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void addAcceptedComponents(int inputNb, std::list<ImageComponents>* comps) OVERRIDE FINAL;
    virtual void addSupportedBitDepth(std::list<ImageBitDepthEnum>* depths) const OVERRIDE FINAL;
    virtual SequentialPreferenceEnum getSequentialPreference() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual StatusEnum getPreferredMetaDatas(NodeMetadata& metadata) OVERRIDE FINAL;
    virtual void onMetaDatasRefreshed(const NodeMetadata& metadata) OVERRIDE FINAL;
    virtual void getComponentsNeededAndProduced(double time, ViewIdx view,
                                                EffectInstance::ComponentsNeededMap* comps,
                                                SequenceTime* passThroughTime,
                                                int* passThroughView,
                                                NodePtr* passThroughInput) OVERRIDE;
    virtual bool isMultiPlanar() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual EffectInstance::PassThroughEnum isPassThroughForNonRenderedPlanes() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isViewAware() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual EffectInstance::ViewInvarianceLevel isViewInvariant() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool supportsConcurrentOpenGLRenders() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual StatusEnum attachOpenGLContext(const OSGLContextPtr& glContext, EffectOpenGLContextDataPtr* data) OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual StatusEnum dettachOpenGLContext(const OSGLContextPtr& glContext, const EffectOpenGLContextDataPtr& data) OVERRIDE FINAL;
    virtual EffectInstancePtr createRenderClone() OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void onInteractViewportSelectionCleared() OVERRIDE FINAL;
    virtual void onInteractViewportSelectionUpdated(const RectD& rectangle, bool onRelease) OVERRIDE FINAL;
    virtual void setInteractColourPicker(const OfxRGBAColourD& color, bool setColor, bool hasColor) OVERRIDE FINAL;
public:

    virtual bool getCanTransform() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool getInputsHoldingTransform(std::list<int>* inputs) const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual StatusEnum getTransform(double time,
                                    const RenderScale & renderScale,
                                    ViewIdx view,
                                    EffectInstancePtr* inputToTransform,
                                    Transform::Matrix3x3* transform) OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isHostMaskingEnabled() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual bool isHostMixingEnabled() const OVERRIDE FINAL WARN_UNUSED_RETURN;
    virtual void onEnableOpenGLKnobValueChanged(bool activated) OVERRIDE FINAL;

    /********OVERRIDEN FROM EFFECT INSTANCE: END*************/
    OfxClipInstance* getClipCorrespondingToInput(int inputNo) const;
    static ContextEnum mapToContextEnum(const std::string &s);
    static std::string mapContextToString(ContextEnum ctx);
    static std::string natronValueChangedReasonToOfxValueChangedReason(ValueChangedReasonEnum reason);

    int getClipInputNumber(const OfxClipInstance* clip) const;

    void onClipLabelChanged(int inputNb, const std::string& label);
    void onClipHintChanged(int inputNb, const std::string& hint);
    void onClipSecretChanged(int inputNb, bool isSecret);
public Q_SLOTS:

    void onSyncPrivateDataRequested();


Q_SIGNALS:

    void syncPrivateDataRequested();

private:


    void tryInitializeOverlayInteracts();

private:

    boost::scoped_ptr<OfxEffectInstancePrivate> _imp;
};

inline AbstractOfxEffectInstancePtr
toAbstractOfxEffectInstance(const EffectInstancePtr& effect)
{
    return boost::dynamic_pointer_cast<AbstractOfxEffectInstance>(effect);
}

inline OfxEffectInstancePtr
toOfxEffectInstance(const EffectInstancePtr& effect)
{
    return boost::dynamic_pointer_cast<OfxEffectInstance>(effect);
}

NATRON_NAMESPACE_EXIT;

#endif // NATRON_ENGINE_OFXNODE_H

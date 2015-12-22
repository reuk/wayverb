#include "ModelRendererComponent.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "combined_config.h"
#include "boundaries.h"
#include "cl_common.h"
#include "tetrahedral_program.h"
#include "waveguide.h"

ModelRendererComponent::ModelRendererComponent()
        : sceneRenderer(std::make_unique<SceneRenderer>())
        , openGLContext(std::make_unique<OpenGLContext>()) {
    openGLContext->setOpenGLVersionRequired(OpenGLContext::openGL3_2);
    openGLContext->setRenderer(sceneRenderer.get());
    openGLContext->setContinuousRepainting(true);
    openGLContext->attachTo(*this);
}

ModelRendererComponent::~ModelRendererComponent() noexcept {
    openGLContext->detach();
}

void ModelRendererComponent::resized() {
    sceneRenderer->setAspect(getLocalBounds().toFloat().getAspectRatio());
}

void ModelRendererComponent::mouseDrag(const MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    auto az = p.x + azimuth;
    auto el = p.y + elevation;
    sceneRenderer->setRotation(az, el);
}

void ModelRendererComponent::mouseUp(const juce::MouseEvent &e) {
    auto p = e.getOffsetFromDragStart().toFloat() * scale;
    azimuth += p.x;
    elevation += p.y;
}

void ModelRendererComponent::filesChanged(ConfigPanel *p,
                                          const File &object,
                                          const File &material,
                                          const File &config) {
    if (object.existsAsFile() && material.existsAsFile() &&
        config.existsAsFile()) {
        sceneData = std::make_unique<SceneData>(
            object.getFullPathName().toStdString(),
            material.getFullPathName().toStdString());
        sceneRenderer->setModelObject(*sceneData);

        CombinedConfig cc;
        try {
            cc = read_config(config.getFullPathName().toStdString());
        } catch (...) {
        }

        MeshBoundary boundary(*sceneData);
        auto context = get_context();
        auto device = get_device(context);
        cl::CommandQueue queue(context, device);
        auto waveguide_program =
            get_program<TetrahedralProgram>(context, device);

        TetrahedralWaveguide waveguide(waveguide_program,
                                       queue,
                                       boundary,
                                       cc.get_divisions(),
                                       cc.get_mic());

        //  TODO so here is the bit where I set up a lovely GL model to draw
        //  out the waveguide pressures and so forth

        auto mic_index = waveguide.get_index_for_coordinate(cc.get_mic());
        auto steps = 1 << 8;
        auto w_results = waveguide.run_gaussian(
            cc.get_source(), mic_index, steps, cc.get_waveguide_sample_rate());

        sceneRenderer->setConfig(
            read_config(config.getFullPathName().toStdString()));
    }
}
#include "ui.h"
#include "uiconfigloader.h"
#include "../render/vertex_buffer.h"
#include "../render/renderer.h"

UI::UI(FileSystem & fsys) :
    m_fsys(fsys), m_fonts(fsys), m_win_buf(VertexBuffer::pos_tex), m_text_win_buf(VertexBuffer::pos_tex)
{
    m_packer = std::make_unique<ChainsPacker>();
}

void UI::update(float time)
{
    for(auto & ptr : m_windows)
    {
        ptr->update(time, true);
    }
}

void UI::clearAndFillBuffers(VertexBuffer & background, VertexBuffer & text) const
{
    background.clear();
    text.clear();

    for(auto const & ptr : m_windows)
    {
        ptr->fillBuffers(background, text);
    }
}

bool UI::init(RendererBase & render)
{
    if(auto file = m_fsys.getFile("ui/jsons/ui_res.json"); file)
    {
        UIImageManagerDesc::ParseUIRes(m_ui_image_atlas, *file, m_fsys);
        FontDataDesc::ParseFontsRes(m_fonts, *file);
        UIDesc::ParseDefaultUISetID(*this, *file);
    }
    else
        return false;

    AtlasTex::UploadAtlasTexture(render, getUIImageAtlas());
    AtlasTex::UploadAtlasTexture(render, getFontImageAtlas());

    return true;
}

void UI::draw(RendererBase & render)
{
    glm::mat4   prj_mtx;
    TextureSlot slot;

    prj_mtx = glm::ortho(0.f, static_cast<float>(m_screen_size.x), 0.f, static_cast<float>(m_screen_size.y),
                         -1.f, 1.f);
    render.setMatrix(RendererBase::MatrixType::PROJECTION, prj_mtx);
    render.setIdentityMatrix(RendererBase::MatrixType::MODELVIEW);

    clearAndFillBuffers(m_win_buf, m_text_win_buf);
    render.uploadBuffer(m_win_buf);
    render.uploadBuffer(m_text_win_buf);

    AlphaState blend;
    DepthState depth;
    depth.enabled       = false;
    blend.blend_enabled = true;

    auto const old_depth = render.setDepthState(depth);
    auto const old_blend = render.setAlphaState(blend);

    // draw background
    slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
    slot.tex_channel_num   = 0;
    slot.texture           = getUIImageAtlas().getAtlasTextureState();
    slot.projector         = nullptr;
    slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
    render.addTextureSlot(slot);
    render.bindSlots();
    render.bindVertexBuffer(&m_win_buf);
    render.draw(m_win_buf);
    render.unbindVertexBuffer();
    render.unbindAndClearSlots();

    // draw text
    render.setDrawColor(getFontColor());
    slot.coord_source      = TextureSlot::TexCoordSource::TEX_COORD_BUFFER;
    slot.tex_channel_num   = 0;
    slot.texture           = getFontImageAtlas().getAtlasTextureState();
    slot.projector         = nullptr;
    slot.combine_mode.mode = CombineStage::CombineMode::MODULATE;
    render.addTextureSlot(slot);
    render.bindSlots();
    render.bindVertexBuffer(&m_text_win_buf);
    render.draw(m_text_win_buf);
    render.unbindVertexBuffer();
    render.unbindAndClearSlots();
    render.setDrawColor(ColorMap::white);   // return to default color

    render.setDepthState(old_depth);
    render.setAlphaState(old_blend);
}

void UI::terminate(RendererBase & render)
{
    AtlasTex::DeleteAtlasTexture(render, getUIImageAtlas());
    AtlasTex::DeleteAtlasTexture(render, getFontImageAtlas());

    render.unloadBuffer(m_win_buf);
    render.deleteBuffer(m_win_buf);

    render.unloadBuffer(m_text_win_buf);
    render.deleteBuffer(m_text_win_buf);
}

UIWindow * UI::loadWindow(InFile & file_json, int32_t layer, std::string const & image_group)
{
    std::unique_ptr<UIWindow> win;
    if(image_group.empty())
        win = std::make_unique<UIWindow>(*this, m_current_gui_set);
    else
        win = std::make_unique<UIWindow>(*this, image_group);

    WindowDesc::LoadWindow(*win, file_json);

    m_windows.push_back(std::move(win));

    auto * win_ptr = m_windows.back().get();
    fitWidgets(win_ptr);

    if(layer + 1 > static_cast<int32_t>(m_layers.size()))
        m_layers.resize(layer + 1);
    // fit windows on layer
    m_layers[layer].push_back(win_ptr);

    return win_ptr;
}

void UI::fitWidgets(UIWindow * win_ptr) const
{
    if(win_ptr == nullptr)
        return;

    m_packer->setSpacing(win_ptr->getSpacing());
    m_packer->fitWidgets(win_ptr);
}

#ifndef UNICODE
#define UNICODE
#endif
#ifndef _UNICODE
#define _UNICODE
#endif

#include <windows.h>
#include <windowsx.h>
#include <commdlg.h>
#include <gdiplus.h>

#include <algorithm>
#include <climits>
#include <cmath>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Gdiplus::Bitmap;
using Gdiplus::Color;
using Gdiplus::CompositingModeSourceCopy;
using Gdiplus::Graphics;
using Gdiplus::Image;
using Gdiplus::Pen;
using Gdiplus::Rect;
using Gdiplus::SolidBrush;

enum class Tool {
    Select,
    Pencil,
    Eraser
};

enum class BrushShape {
    Round,
    Square
};

struct Part {
    std::string id;
    std::string label;
    int x = 0;
    int y = 0;
    int w = 32;
    int h = 32;
};

struct TextureTemplate {
    std::string id;
    std::string name;
    std::string description;
    std::string default_image;
    int width = 0;
    int height = 0;
    std::vector<Part> parts;
};

struct AppState {
    fs::path root;
    fs::path template_path;
    fs::path current_image_path;
    std::vector<TextureTemplate> templates;
    int selected_template = 0;
    int selected_part = 0;
    bool dev_mode = false;
    bool show_all_parts = false;
    bool fit_to_view = true;
    bool drawing = false;
    bool panning = false;
    bool right_erasing = false;
    Tool tool = Tool::Select;
    int brush_size = 8;
    int brush_hardness = 100;
    BrushShape brush_shape = BrushShape::Round;
    COLORREF brush_color = RGB(255, 255, 255);
    double zoom = 1.0;
    int pan_x = 0;
    int pan_y = 0;
    POINT last_mouse{};
    POINT hover_mouse{};
    bool hover_canvas = false;
    bool tracking_mouse = false;
    bool tool_panel_open = false;
    double part_scale_x = 1.0;
    double part_scale_y = 1.0;
    std::unique_ptr<Bitmap> image;
    std::vector<std::unique_ptr<Bitmap>> undo_stack;
    std::vector<std::unique_ptr<Bitmap>> redo_stack;
    Rect image_rect;
    Rect preview_rect;
    Rect tool_panel_rect;
    ULONG_PTR gdiplus_token = 0;
};

AppState g_app;
HWND g_main = nullptr;
HWND g_templates = nullptr;
HWND g_parts = nullptr;
HWND g_status = nullptr;
HWND g_dev_check = nullptr;
HWND g_id = nullptr;
HWND g_label = nullptr;
HWND g_x = nullptr;
HWND g_y = nullptr;
HWND g_w = nullptr;
HWND g_h = nullptr;
HWND g_apply = nullptr;
HWND g_new_part = nullptr;
HWND g_delete_part = nullptr;
HWND g_part_up = nullptr;
HWND g_part_down = nullptr;
HWND g_save = nullptr;
HWND g_open = nullptr;
HWND g_save_png = nullptr;
HWND g_reset = nullptr;
HWND g_show_all = nullptr;
HWND g_select_tool = nullptr;
HWND g_pencil_tool = nullptr;
HWND g_eraser_tool = nullptr;
HWND g_color_pick = nullptr;
HWND g_zoom_out = nullptr;
HWND g_zoom_label = nullptr;
HWND g_zoom_in = nullptr;
HWND g_zoom_fit = nullptr;
HWND g_brush_size = nullptr;
HWND g_options = nullptr;
HWND g_tool_preview = nullptr;
HFONT g_ui_font = nullptr;
WNDPROC g_edit_proc = nullptr;
WNDPROC g_parts_proc = nullptr;
bool g_parts_suppress_click = false;
HBRUSH g_panel_brush = nullptr;
HBRUSH g_edit_brush = nullptr;
HWND g_options_window = nullptr;
HWND g_options_size = nullptr;
HWND g_options_hardness = nullptr;
HWND g_shape_round = nullptr;
HWND g_shape_square = nullptr;
HWND g_options_color = nullptr;
HWND g_options_close = nullptr;

constexpr int ID_TEMPLATES = 1001;
constexpr int ID_PARTS = 1002;
constexpr int ID_OPEN = 1003;
constexpr int ID_RESET = 1004;
constexpr int ID_DEV = 1005;
constexpr int ID_APPLY = 1006;
constexpr int ID_NEW = 1007;
constexpr int ID_DELETE = 1008;
constexpr int ID_SAVE = 1009;
constexpr int ID_SHOW_ALL = 1010;
constexpr int ID_SAVE_PNG = 1011;
constexpr int ID_TOOL_SELECT = 1012;
constexpr int ID_TOOL_PENCIL = 1013;
constexpr int ID_TOOL_ERASER = 1014;
constexpr int ID_ZOOM_OUT = 1015;
constexpr int ID_ZOOM_IN = 1016;
constexpr int ID_ZOOM_FIT = 1017;
constexpr int ID_OPTIONS = 1018;
constexpr int ID_TOOL_PREVIEW = 1019;
constexpr int ID_OPTIONS_CLOSE = 1020;
constexpr int ID_COLOR_PICK = 1021;
constexpr int ID_SHAPE_ROUND = 1022;
constexpr int ID_SHAPE_SQUARE = 1023;
constexpr int ID_HARDNESS = 1024;
constexpr int ID_BRUSH_SIZE = 1025;
constexpr int ID_OPTIONS_SIZE = 1026;
constexpr int ID_OPTIONS_COLOR = 1027;
constexpr int ID_PART_UP = 1028;
constexpr int ID_PART_DOWN = 1029;

constexpr size_t MAX_HISTORY = 400;

constexpr int UI_MENU_H = 34;
constexpr int UI_OPTIONS_H = 44;
constexpr int UI_STATUS_H = 28;
constexpr int UI_TOOLBAR_W = 58;
constexpr int UI_LEFT_PANEL_W = 266;
constexpr int UI_RIGHT_PANEL_W = 306;
constexpr int UI_GAP = 10;

void invalidate_tool_ui();
void choose_brush_color(HWND owner);
void undo_image();
void redo_image();
void subclass_edit(HWND hwnd);
void populate_dev_fields();
void close_tool_panel();
void set_status(const std::wstring& message);
void layout_tool_panel_controls(HWND hwnd);

std::wstring widen(const std::string& text) {
    if (text.empty()) {
        return L"";
    }
    int size = MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    std::wstring wide(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), wide.data(), size);
    return wide;
}

std::string narrow(const std::wstring& text) {
    if (text.empty()) {
        return "";
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    std::string utf8(size, '\0');
    WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), utf8.data(), size, nullptr, nullptr);
    return utf8;
}

std::wstring read_control(HWND hwnd) {
    int len = GetWindowTextLengthW(hwnd);
    std::wstring text(len, L'\0');
    GetWindowTextW(hwnd, text.data(), len + 1);
    return text;
}

void set_control(HWND hwnd, const std::string& text) {
    SetWindowTextW(hwnd, widen(text).c_str());
}

void set_control_int(HWND hwnd, int value) {
    SetWindowTextW(hwnd, std::to_wstring(value).c_str());
}

void set_control_int_if_changed(HWND hwnd, int value) {
    if (!hwnd) {
        return;
    }
    std::wstring next = std::to_wstring(value);
    if (read_control(hwnd) != next) {
        SetWindowTextW(hwnd, next.c_str());
    }
}

void delete_previous_word(HWND hwnd) {
    DWORD start = 0;
    DWORD end = 0;
    SendMessageW(hwnd, EM_GETSEL, reinterpret_cast<WPARAM>(&start), reinterpret_cast<LPARAM>(&end));
    if (start != end) {
        SendMessageW(hwnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(L""));
        return;
    }

    int len = GetWindowTextLengthW(hwnd);
    std::wstring text(len, L'\0');
    GetWindowTextW(hwnd, text.data(), len + 1);
    int pos = static_cast<int>(start);
    while (pos > 0 && iswspace(text[pos - 1])) {
        --pos;
    }
    while (pos > 0 && !iswspace(text[pos - 1])) {
        --pos;
    }
    SendMessageW(hwnd, EM_SETSEL, pos, start);
    SendMessageW(hwnd, EM_REPLACESEL, TRUE, reinterpret_cast<LPARAM>(L""));
}

LRESULT CALLBACK edit_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
        if (wparam == 'A') {
            SendMessageW(hwnd, EM_SETSEL, 0, -1);
            return 0;
        }
        if (wparam == VK_BACK) {
            delete_previous_word(hwnd);
            return 0;
        }
        if (wparam == 'Z') {
            undo_image();
            return 0;
        }
        if (wparam == 'Y') {
            redo_image();
            return 0;
        }
    }
    return CallWindowProcW(g_edit_proc, hwnd, message, wparam, lparam);
}

void close_tool_panel() {
    if (!g_app.tool_panel_open) {
        return;
    }
    g_app.tool_panel_open = false;
    HWND controls[] = {g_options_size, g_options_hardness, g_shape_round, g_shape_square, g_options_color, g_options_close};
    for (HWND control : controls) {
        if (control) {
            ShowWindow(control, SW_HIDE);
        }
    }
    InvalidateRect(g_main, nullptr, FALSE);
    set_status(L"Painel da ferramenta fechado.");
}

LRESULT CALLBACK parts_list_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_LBUTTONDOWN) {
        close_tool_panel();
        DWORD hit = static_cast<DWORD>(SendMessageW(hwnd, LB_ITEMFROMPOINT, 0, lparam));
        int index = LOWORD(hit);
        bool outside = HIWORD(hit) != 0;
        if (!outside && index == g_app.selected_part) {
            g_parts_suppress_click = true;
            g_app.selected_part = -1;
            SendMessageW(hwnd, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
            populate_dev_fields();
            InvalidateRect(hwnd, nullptr, TRUE);
            InvalidateRect(g_main, nullptr, FALSE);
            set_status(L"Nenhuma parte selecionada. Editando a imagem inteira.");
            return 0;
        }
    }
    if (message == WM_LBUTTONUP && g_parts_suppress_click) {
        g_parts_suppress_click = false;
        SendMessageW(hwnd, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
        InvalidateRect(hwnd, nullptr, TRUE);
        return 0;
    }
    return CallWindowProcW(g_parts_proc, hwnd, message, wparam, lparam);
}

int get_control_int(HWND hwnd, int fallback) {
    try {
        return std::stoi(read_control(hwnd));
    } catch (...) {
        return fallback;
    }
}

int clamp_int(int value, int min_value, int max_value) {
    return std::max(min_value, std::min(max_value, value));
}

RECT to_win_rect(const Rect& rect) {
    return RECT{rect.X, rect.Y, rect.X + rect.Width, rect.Y + rect.Height};
}

RECT inflated_win_rect(const Rect& rect, int inflate) {
    RECT out = to_win_rect(rect);
    InflateRect(&out, inflate, inflate);
    return out;
}

int get_encoder_clsid(const WCHAR* format, CLSID* clsid) {
    UINT count = 0;
    UINT size = 0;
    Gdiplus::GetImageEncodersSize(&count, &size);
    if (size == 0) {
        return -1;
    }
    std::vector<unsigned char> buffer(size);
    auto* info = reinterpret_cast<Gdiplus::ImageCodecInfo*>(buffer.data());
    Gdiplus::GetImageEncoders(count, size, info);
    for (UINT i = 0; i < count; ++i) {
        if (wcscmp(info[i].MimeType, format) == 0) {
            *clsid = info[i].Clsid;
            return static_cast<int>(i);
        }
    }
    return -1;
}

const wchar_t* tool_name(Tool tool) {
    switch (tool) {
    case Tool::Select:
        return L"Selecionar";
    case Tool::Pencil:
        return L"Lapis";
    case Tool::Eraser:
        return L"Borracha";
    }
    return L"Ferramenta";
}

const wchar_t* brush_shape_name(BrushShape shape) {
    return shape == BrushShape::Square ? L"Quadrado" : L"Redondo";
}

std::string read_file(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Nao foi possivel abrir " + path.string());
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

void write_file(const fs::path& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Nao foi possivel salvar " + path.string());
    }
    file << content;
}

fs::path find_project_root_from(fs::path current) {
    while (!current.empty()) {
        if (fs::exists(current / "data" / "templates" / "teeworlds_textures.json")) {
            return current;
        }
        fs::path parent = current.parent_path();
        if (parent == current) {
            break;
        }
        current = parent;
    }
    return {};
}

fs::path app_root() {
    if (fs::path found = find_project_root_from(fs::current_path()); !found.empty()) {
        return found;
    }
    wchar_t module_path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, module_path, MAX_PATH);
    return find_project_root_from(fs::path(module_path).parent_path());
}

size_t matching_char(const std::string& text, size_t open_pos, char open, char close) {
    int depth = 0;
    bool in_string = false;
    bool escaped = false;
    for (size_t i = open_pos; i < text.size(); ++i) {
        char c = text[i];
        if (in_string) {
            if (escaped) {
                escaped = false;
            } else if (c == '\\') {
                escaped = true;
            } else if (c == '"') {
                in_string = false;
            }
            continue;
        }
        if (c == '"') {
            in_string = true;
        } else if (c == open) {
            ++depth;
        } else if (c == close) {
            --depth;
            if (depth == 0) {
                return i;
            }
        }
    }
    return std::string::npos;
}

std::string json_string(const std::string& block, const std::string& key, const std::string& fallback = "") {
    std::regex pattern("\"" + key + R"REGEX("\s*:\s*"([^"]*)")REGEX");
    std::smatch match;
    if (std::regex_search(block, match, pattern)) {
        return match[1].str();
    }
    return fallback;
}

int json_int(const std::string& block, const std::string& key, int fallback = 0) {
    std::regex pattern("\"" + key + R"("\s*:\s*(-?\d+))");
    std::smatch match;
    if (std::regex_search(block, match, pattern)) {
        return std::stoi(match[1].str());
    }
    return fallback;
}

std::vector<Part> parse_parts(const std::string& template_block) {
    std::vector<Part> parts;
    size_t parts_key = template_block.find("\"parts\"");
    if (parts_key == std::string::npos) {
        return parts;
    }
    size_t array_open = template_block.find('[', parts_key);
    size_t array_close = matching_char(template_block, array_open, '[', ']');
    if (array_open == std::string::npos || array_close == std::string::npos) {
        return parts;
    }
    std::string array = template_block.substr(array_open + 1, array_close - array_open - 1);
    size_t pos = 0;
    while ((pos = array.find('{', pos)) != std::string::npos) {
        size_t end = matching_char(array, pos, '{', '}');
        if (end == std::string::npos) {
            break;
        }
        std::string block = array.substr(pos, end - pos + 1);
        Part part;
        part.id = json_string(block, "id", "part");
        part.label = json_string(block, "label", part.id);
        part.x = json_int(block, "x");
        part.y = json_int(block, "y");
        part.w = json_int(block, "w", 32);
        part.h = json_int(block, "h", 32);
        parts.push_back(part);
        pos = end + 1;
    }
    return parts;
}

std::vector<TextureTemplate> parse_templates(const std::string& json) {
    std::vector<TextureTemplate> templates;
    size_t key = json.find("\"templates\"");
    size_t array_open = json.find('[', key);
    size_t array_close = matching_char(json, array_open, '[', ']');
    if (key == std::string::npos || array_open == std::string::npos || array_close == std::string::npos) {
        return templates;
    }
    std::string array = json.substr(array_open + 1, array_close - array_open - 1);
    size_t pos = 0;
    while ((pos = array.find('{', pos)) != std::string::npos) {
        size_t end = matching_char(array, pos, '{', '}');
        if (end == std::string::npos) {
            break;
        }
        std::string block = array.substr(pos, end - pos + 1);
        TextureTemplate texture;
        texture.id = json_string(block, "id");
        texture.name = json_string(block, "name", texture.id);
        texture.description = json_string(block, "description");
        texture.default_image = json_string(block, "default_image");
        texture.width = json_int(block, "width");
        texture.height = json_int(block, "height");
        texture.parts = parse_parts(block);
        if (!texture.id.empty()) {
            templates.push_back(texture);
        }
        pos = end + 1;
    }
    return templates;
}

std::string escape_json(const std::string& value) {
    std::string out;
    for (char c : value) {
        if (c == '"' || c == '\\') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    return out;
}

std::string serialize_templates(const std::vector<TextureTemplate>& templates) {
    std::ostringstream out;
    out << "{\n  \"templates\": [\n";
    for (size_t i = 0; i < templates.size(); ++i) {
        const auto& texture = templates[i];
        out << "    {\n";
        out << "      \"id\": \"" << escape_json(texture.id) << "\",\n";
        out << "      \"name\": \"" << escape_json(texture.name) << "\",\n";
        out << "      \"description\": \"" << escape_json(texture.description) << "\",\n";
        out << "      \"default_image\": \"" << escape_json(texture.default_image) << "\",\n";
        out << "      \"width\": " << texture.width << ",\n";
        out << "      \"height\": " << texture.height << ",\n";
        out << "      \"parts\": [\n";
        for (size_t p = 0; p < texture.parts.size(); ++p) {
            const auto& part = texture.parts[p];
            out << "        { \"id\": \"" << escape_json(part.id)
                << "\", \"label\": \"" << escape_json(part.label)
                << "\", \"x\": " << part.x
                << ", \"y\": " << part.y
                << ", \"w\": " << part.w
                << ", \"h\": " << part.h << " }";
            out << (p + 1 == texture.parts.size() ? "\n" : ",\n");
        }
        out << "      ]\n";
        out << "    }" << (i + 1 == templates.size() ? "\n" : ",\n");
    }
    out << "  ]\n}\n";
    return out.str();
}

TextureTemplate* current_template() {
    if (g_app.selected_template < 0 || g_app.selected_template >= static_cast<int>(g_app.templates.size())) {
        return nullptr;
    }
    return &g_app.templates[g_app.selected_template];
}

Part* current_part() {
    TextureTemplate* texture = current_template();
    if (!texture || g_app.selected_part < 0 || g_app.selected_part >= static_cast<int>(texture->parts.size())) {
        return nullptr;
    }
    return &texture->parts[g_app.selected_part];
}

void set_status(const std::wstring& message) {
    SetWindowTextW(g_status, message.c_str());
}

std::unique_ptr<Bitmap> clone_bitmap(Bitmap* source) {
    if (!source) {
        return nullptr;
    }
    Rect full(0, 0, static_cast<int>(source->GetWidth()), static_cast<int>(source->GetHeight()));
    std::unique_ptr<Bitmap> copy(source->Clone(full, PixelFormat32bppARGB));
    if (!copy || copy->GetLastStatus() != Gdiplus::Ok) {
        return nullptr;
    }
    return copy;
}

void trim_history(std::vector<std::unique_ptr<Bitmap>>& stack) {
    while (stack.size() > MAX_HISTORY) {
        stack.erase(stack.begin());
    }
}

void clear_history() {
    g_app.undo_stack.clear();
    g_app.redo_stack.clear();
}

void push_undo_snapshot() {
    if (!g_app.image) {
        return;
    }
    std::unique_ptr<Bitmap> snapshot = clone_bitmap(g_app.image.get());
    if (!snapshot) {
        set_status(L"Nao foi possivel criar snapshot de undo.");
        return;
    }
    g_app.undo_stack.push_back(std::move(snapshot));
    trim_history(g_app.undo_stack);
    g_app.redo_stack.clear();
}

void undo_image() {
    if (!g_app.image || g_app.undo_stack.empty()) {
        set_status(L"Nada para desfazer.");
        return;
    }
    std::unique_ptr<Bitmap> current = clone_bitmap(g_app.image.get());
    if (current) {
        g_app.redo_stack.push_back(std::move(current));
        trim_history(g_app.redo_stack);
    }
    g_app.image = std::move(g_app.undo_stack.back());
    g_app.undo_stack.pop_back();
    InvalidateRect(g_main, nullptr, FALSE);
    set_status(L"Ctrl+Z aplicado.");
}

void redo_image() {
    if (!g_app.image || g_app.redo_stack.empty()) {
        set_status(L"Nada para refazer.");
        return;
    }
    std::unique_ptr<Bitmap> current = clone_bitmap(g_app.image.get());
    if (current) {
        g_app.undo_stack.push_back(std::move(current));
        trim_history(g_app.undo_stack);
    }
    g_app.image = std::move(g_app.redo_stack.back());
    g_app.redo_stack.pop_back();
    InvalidateRect(g_main, nullptr, FALSE);
    set_status(L"Ctrl+Y aplicado.");
}

void load_image(const fs::path& path, bool force = false) {
    std::error_code ignored;
    if (!force && !g_app.current_image_path.empty() && fs::equivalent(g_app.current_image_path, path, ignored)) {
        return;
    }
    g_app.current_image_path = path;
    std::unique_ptr<Bitmap> loaded(Bitmap::FromFile(path.wstring().c_str()));
    if (!loaded || loaded->GetLastStatus() != Gdiplus::Ok) {
        g_app.image.reset();
        set_status(L"Falha ao carregar imagem.");
        return;
    }
    Rect full(0, 0, static_cast<int>(loaded->GetWidth()), static_cast<int>(loaded->GetHeight()));
    g_app.image.reset(loaded->Clone(full, PixelFormat32bppARGB));
    if (!g_app.image || g_app.image->GetLastStatus() != Gdiplus::Ok) {
        g_app.image.reset();
        set_status(L"Falha ao preparar imagem para edicao.");
        return;
    }
    g_app.fit_to_view = true;
    g_app.zoom = 1.0;
    g_app.pan_x = 0;
    g_app.pan_y = 0;
    if (TextureTemplate* texture = current_template(); texture && texture->width > 0 && texture->height > 0) {
        g_app.part_scale_x = static_cast<double>(g_app.image->GetWidth()) / texture->width;
        g_app.part_scale_y = static_cast<double>(g_app.image->GetHeight()) / texture->height;
    } else {
        g_app.part_scale_x = 1.0;
        g_app.part_scale_y = 1.0;
    }
    clear_history();
    std::wstring size = std::to_wstring(g_app.image->GetWidth()) + L"x" + std::to_wstring(g_app.image->GetHeight());
    set_status(L"Imagem carregada: " + path.filename().wstring() + L" (" + size + L")");
    InvalidateRect(g_main, nullptr, FALSE);
}

void save_image_as(HWND owner) {
    if (!g_app.image) {
        return;
    }
    wchar_t file_name[MAX_PATH] = {};
    wcscpy_s(file_name, L"texture-edited.png");
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"PNG files (*.png)\0*.png\0";
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = L"png";
    if (!GetSaveFileNameW(&ofn)) {
        return;
    }
    CLSID png_clsid{};
    if (get_encoder_clsid(L"image/png", &png_clsid) < 0) {
        set_status(L"Encoder PNG nao encontrado.");
        return;
    }
    if (g_app.image->Save(file_name, &png_clsid, nullptr) == Gdiplus::Ok) {
        set_status(L"PNG salvo: " + fs::path(file_name).filename().wstring());
    } else {
        set_status(L"Falha ao salvar PNG.");
    }
}

void load_default_image(bool force = false) {
    TextureTemplate* texture = current_template();
    if (!texture) {
        return;
    }
    fs::path path = g_app.root / texture->default_image;
    load_image(path, force);
}

void populate_dev_fields() {
    Part* part = current_part();
    if (!part) {
        set_control(g_id, "");
        set_control(g_label, "");
        set_control_int(g_x, 0);
        set_control_int(g_y, 0);
        set_control_int(g_w, 0);
        set_control_int(g_h, 0);
        return;
    }
    set_control(g_id, part->id);
    set_control(g_label, part->label);
    set_control_int(g_x, part->x);
    set_control_int(g_y, part->y);
    set_control_int(g_w, part->w);
    set_control_int(g_h, part->h);
}

void rebuild_parts() {
    SendMessageW(g_parts, LB_RESETCONTENT, 0, 0);
    TextureTemplate* texture = current_template();
    if (!texture) {
        return;
    }
    for (const Part& part : texture->parts) {
        std::wstring item = widen(part.id + " - " + part.label);
        SendMessageW(g_parts, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
    }
    if (texture->parts.empty()) {
        g_app.selected_part = -1;
    } else if (g_app.selected_part >= static_cast<int>(texture->parts.size())) {
        g_app.selected_part = static_cast<int>(texture->parts.size()) - 1;
    }
    if (g_app.selected_part >= 0) {
        SendMessageW(g_parts, LB_SETCURSEL, g_app.selected_part, 0);
    } else {
        SendMessageW(g_parts, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
    }
    populate_dev_fields();
    InvalidateRect(g_main, nullptr, FALSE);
}

void move_selected_part(int direction) {
    TextureTemplate* texture = current_template();
    if (!texture || g_app.selected_part < 0 || g_app.selected_part >= static_cast<int>(texture->parts.size())) {
        set_status(L"Selecione uma parte para reordenar.");
        return;
    }
    int next = g_app.selected_part + direction;
    if (next < 0 || next >= static_cast<int>(texture->parts.size())) {
        return;
    }
    std::swap(texture->parts[g_app.selected_part], texture->parts[next]);
    g_app.selected_part = next;
    rebuild_parts();
    set_status(L"Ordem da parte alterada. Clique em Salvar JSON para gravar.");
}

void rebuild_templates() {
    SendMessageW(g_templates, LB_RESETCONTENT, 0, 0);
    for (const auto& texture : g_app.templates) {
        std::wstring item = widen(texture.id + " - " + texture.name);
        SendMessageW(g_templates, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
    }
    if (!g_app.templates.empty()) {
        SendMessageW(g_templates, LB_SETCURSEL, g_app.selected_template, 0);
    }
    rebuild_parts();
}

void apply_dev_fields() {
    Part* part = current_part();
    if (!part) {
        return;
    }
    part->id = narrow(read_control(g_id));
    part->label = narrow(read_control(g_label));
    part->x = get_control_int(g_x, part->x);
    part->y = get_control_int(g_y, part->y);
    part->w = std::max(1, get_control_int(g_w, part->w));
    part->h = std::max(1, get_control_int(g_h, part->h));
    rebuild_parts();
    set_status(L"Parte atualizada no modo-dev. Clique em Salvar JSON para gravar.");
}

void save_templates() {
    apply_dev_fields();
    write_file(g_app.template_path, serialize_templates(g_app.templates));
    set_status(L"JSON salvo em data/templates/teeworlds_textures.json");
}

void add_part() {
    TextureTemplate* texture = current_template();
    if (!texture) {
        return;
    }
    Part part;
    part.id = "new_part";
    part.label = "New Part";
    part.x = 0;
    part.y = 0;
    part.w = std::max(32, texture->width / 10);
    part.h = std::max(32, texture->height / 10);
    texture->parts.push_back(part);
    g_app.selected_part = static_cast<int>(texture->parts.size()) - 1;
    rebuild_parts();
    set_status(L"Nova parte criada. Ajuste os campos e salve o JSON.");
}

void delete_part() {
    TextureTemplate* texture = current_template();
    if (!texture || g_app.selected_part < 0 || g_app.selected_part >= static_cast<int>(texture->parts.size())) {
        return;
    }
    texture->parts.erase(texture->parts.begin() + g_app.selected_part);
    g_app.selected_part = std::min(g_app.selected_part, static_cast<int>(texture->parts.size()) - 1);
    rebuild_parts();
    set_status(L"Parte removida. Clique em Salvar JSON para gravar.");
}

void choose_image(HWND owner) {
    wchar_t file_name[MAX_PATH] = {};
    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"PNG files (*.png)\0*.png\0All files (*.*)\0*.*\0";
    ofn.lpstrFile = file_name;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    if (GetOpenFileNameW(&ofn)) {
        load_image(file_name);
    }
}

void layout(HWND hwnd) {
    RECT client{};
    GetClientRect(hwnd, &client);
    int width = client.right - client.left;
    int height = client.bottom - client.top;
    int content_top = UI_MENU_H + UI_OPTIONS_H;
    int left_panel_x = UI_TOOLBAR_W;
    int left_panel_w = UI_LEFT_PANEL_W;
    int right_panel_w = UI_RIGHT_PANEL_W;
    int right_x = width - right_panel_w + 12;
    int canvas_x = UI_TOOLBAR_W + UI_LEFT_PANEL_W + UI_GAP;
    int canvas_w = std::max(80, width - canvas_x - right_panel_w - UI_GAP);

    MoveWindow(g_open, 12, 4, 96, 26, TRUE);
    MoveWindow(g_save_png, 114, 4, 104, 26, TRUE);
    MoveWindow(g_reset, 224, 4, 128, 26, TRUE);
    if (g_options) {
        ShowWindow(g_options, SW_HIDE);
    }
    MoveWindow(g_dev_check, width - 126, 4, 116, 26, TRUE);
    MoveWindow(g_zoom_fit, width - 184, 4, 48, 26, TRUE);
    MoveWindow(g_zoom_in, width - 226, 4, 34, 26, TRUE);
    MoveWindow(g_zoom_label, width - 292, 7, 58, 22, TRUE);
    MoveWindow(g_zoom_out, width - 334, 4, 34, 26, TRUE);

    MoveWindow(g_select_tool, 9, content_top + 10, 40, 36, TRUE);
    MoveWindow(g_pencil_tool, 9, content_top + 54, 40, 36, TRUE);
    MoveWindow(g_eraser_tool, 9, content_top + 98, 40, 36, TRUE);

    HWND brush_label = GetDlgItem(hwnd, 2007);
    MoveWindow(brush_label, canvas_x, UI_MENU_H + 12, 58, 22, TRUE);
    MoveWindow(g_brush_size, canvas_x + 62, UI_MENU_H + 9, 58, 26, TRUE);
    MoveWindow(g_color_pick, canvas_x + 132, UI_MENU_H + 7, 92, 30, TRUE);
    MoveWindow(g_show_all, canvas_x + 236, UI_MENU_H + 9, 158, 26, TRUE);

    MoveWindow(g_templates, left_panel_x + 8, content_top + 42, left_panel_w - 16, 148, TRUE);
    MoveWindow(g_parts, left_panel_x + 8, content_top + 222, left_panel_w - 16, height - content_top - 242 - UI_STATUS_H, TRUE);
    MoveWindow(g_status, 0, height - UI_STATUS_H, width, UI_STATUS_H, TRUE);

    int y = content_top + 46;
    auto place = [&](HWND label, HWND edit) {
        MoveWindow(label, right_x, y, 70, 22, TRUE);
        MoveWindow(edit, right_x + 78, y, right_panel_w - 100, 24, TRUE);
        y += 34;
    };

    ShowWindow(brush_label, SW_SHOW);
    ShowWindow(g_brush_size, SW_SHOW);

    ShowWindow(g_tool_preview, SW_SHOW);
    MoveWindow(g_tool_preview, right_x, y, right_panel_w - 24, 136, TRUE);
    y += 154;

    HWND children[] = {g_id, g_label, g_x, g_y, g_w, g_h, g_apply, g_new_part, g_delete_part, g_part_up, g_part_down, g_save};
    for (HWND child : children) {
        ShowWindow(child, g_app.dev_mode ? SW_SHOW : SW_HIDE);
    }

    if (g_app.dev_mode) {
        HWND labels[6] = {};
        labels[0] = GetDlgItem(hwnd, 2001);
        labels[1] = GetDlgItem(hwnd, 2002);
        labels[2] = GetDlgItem(hwnd, 2003);
        labels[3] = GetDlgItem(hwnd, 2004);
        labels[4] = GetDlgItem(hwnd, 2005);
        labels[5] = GetDlgItem(hwnd, 2006);
        HWND brush_label = GetDlgItem(hwnd, 2007);
        for (HWND label : labels) {
            ShowWindow(label, SW_SHOW);
        }
        place(labels[0], g_id);
        place(labels[1], g_label);
        place(labels[2], g_x);
        place(labels[3], g_y);
        place(labels[4], g_w);
        place(labels[5], g_h);
        MoveWindow(g_apply, right_x, y + UI_GAP, right_panel_w - 24, 30, TRUE);
        MoveWindow(g_new_part, right_x, y + 46, (right_panel_w - 34) / 2, 30, TRUE);
        MoveWindow(g_delete_part, right_x + (right_panel_w - 34) / 2 + 10, y + 46, (right_panel_w - 34) / 2, 30, TRUE);
        MoveWindow(g_part_up, right_x, y + 82, (right_panel_w - 34) / 2, 30, TRUE);
        MoveWindow(g_part_down, right_x + (right_panel_w - 34) / 2 + 10, y + 82, (right_panel_w - 34) / 2, 30, TRUE);
        MoveWindow(g_save, right_x, y + 128, right_panel_w - 24, 34, TRUE);
    } else {
        for (int id = 2001; id <= 2006; ++id) {
            ShowWindow(GetDlgItem(hwnd, id), SW_HIDE);
        }
    }
    layout_tool_panel_controls(hwnd);
    InvalidateRect(hwnd, nullptr, TRUE);
}

Rect canvas_rect(HWND hwnd) {
    RECT client{};
    GetClientRect(hwnd, &client);
    int left = UI_TOOLBAR_W + UI_LEFT_PANEL_W + UI_GAP;
    int top = UI_MENU_H + UI_OPTIONS_H;
    int right_panel = UI_RIGHT_PANEL_W;
    return Rect(left, top, std::max(50, static_cast<int>(client.right) - left - right_panel - UI_GAP), std::max(50, static_cast<int>(client.bottom) - top - UI_STATUS_H - UI_GAP));
}

Rect fit_image_rect(const Rect& bounds, Image* image) {
    if (!image) {
        return bounds;
    }
    double iw = static_cast<double>(image->GetWidth());
    double ih = static_cast<double>(image->GetHeight());
    double scale = g_app.fit_to_view ? std::min(1.0, std::min(bounds.Width / iw, bounds.Height / ih)) : g_app.zoom;
    int w = static_cast<int>(iw * scale);
    int h = static_cast<int>(ih * scale);
    return Rect(bounds.X + (bounds.Width - w) / 2 + g_app.pan_x, bounds.Y + (bounds.Height - h) / 2 + g_app.pan_y, w, h);
}

void draw_pixel_grid(Graphics& g) {
    if (!g_app.image || g_app.image_rect.Width <= 0 || g_app.image_rect.Height <= 0) {
        return;
    }

    double step_x = static_cast<double>(g_app.image_rect.Width) / g_app.image->GetWidth();
    double step_y = static_cast<double>(g_app.image_rect.Height) / g_app.image->GetHeight();
    if (step_x < 8.0 || step_y < 8.0) {
        return;
    }

    Pen grid_pen(Color(70, 255, 255, 255), 1.0f);
    int image_w = static_cast<int>(g_app.image->GetWidth());
    int image_h = static_cast<int>(g_app.image->GetHeight());
    for (int x = 0; x <= image_w; ++x) {
        int sx = g_app.image_rect.X + static_cast<int>(std::round(x * step_x));
        g.DrawLine(&grid_pen, sx, g_app.image_rect.Y, sx, g_app.image_rect.Y + g_app.image_rect.Height);
    }
    for (int y = 0; y <= image_h; ++y) {
        int sy = g_app.image_rect.Y + static_cast<int>(std::round(y * step_y));
        g.DrawLine(&grid_pen, g_app.image_rect.X, sy, g_app.image_rect.X + g_app.image_rect.Width, sy);
    }
}

void draw_image_checker(Graphics& g) {
    if (!g_app.image || g_app.image_rect.Width <= 0 || g_app.image_rect.Height <= 0) {
        return;
    }

    double scale_x = static_cast<double>(g_app.image_rect.Width) / g_app.image->GetWidth();
    double scale_y = static_cast<double>(g_app.image_rect.Height) / g_app.image->GetHeight();
    int source_cell = 8;
    if (std::min(scale_x, scale_y) < 0.75) {
        source_cell = 16;
    }
    if (std::min(scale_x, scale_y) < 0.35) {
        source_cell = 32;
    }

    SolidBrush checker_a(Color(255, 31, 36, 46));
    SolidBrush checker_b(Color(255, 42, 48, 60));
    int image_w = static_cast<int>(g_app.image->GetWidth());
    int image_h = static_cast<int>(g_app.image->GetHeight());

    g.SetClip(g_app.image_rect);
    for (int sy = 0; sy < image_h; sy += source_cell) {
        for (int sx = 0; sx < image_w; sx += source_cell) {
            int left = g_app.image_rect.X + static_cast<int>(std::floor(sx * scale_x));
            int top = g_app.image_rect.Y + static_cast<int>(std::floor(sy * scale_y));
            int right = g_app.image_rect.X + static_cast<int>(std::ceil(std::min(sx + source_cell, image_w) * scale_x));
            int bottom = g_app.image_rect.Y + static_cast<int>(std::ceil(std::min(sy + source_cell, image_h) * scale_y));
            bool alt = ((sx / source_cell) + (sy / source_cell)) % 2 == 0;
            g.FillRectangle(alt ? &checker_a : &checker_b, left, top, std::max(1, right - left), std::max(1, bottom - top));
        }
    }
    g.ResetClip();
}

POINT screen_to_image_point(int x, int y) {
    POINT point{-1, -1};
    if (!g_app.image || g_app.image_rect.Width <= 0 || g_app.image_rect.Height <= 0) {
        return point;
    }
    double sx = static_cast<double>(g_app.image->GetWidth()) / g_app.image_rect.Width;
    double sy = static_cast<double>(g_app.image->GetHeight()) / g_app.image_rect.Height;
    point.x = static_cast<LONG>((x - g_app.image_rect.X) * sx);
    point.y = static_cast<LONG>((y - g_app.image_rect.Y) * sy);
    return point;
}

bool image_point_inside(const POINT& point) {
    return g_app.image && point.x >= 0 && point.y >= 0 && point.x < static_cast<LONG>(g_app.image->GetWidth()) && point.y < static_cast<LONG>(g_app.image->GetHeight());
}

Rect part_to_screen(const Part& part) {
    Image* image = g_app.image.get();
    if (!image || g_app.image_rect.Width <= 0 || g_app.image_rect.Height <= 0) {
        return Rect();
    }
    double sx = static_cast<double>(g_app.image_rect.Width) / image->GetWidth();
    double sy = static_cast<double>(g_app.image_rect.Height) / image->GetHeight();
    int px = static_cast<int>(std::round(part.x * g_app.part_scale_x));
    int py = static_cast<int>(std::round(part.y * g_app.part_scale_y));
    int pw = std::max(1, static_cast<int>(std::round(part.w * g_app.part_scale_x)));
    int ph = std::max(1, static_cast<int>(std::round(part.h * g_app.part_scale_y)));
    return Rect(
        g_app.image_rect.X + static_cast<int>(px * sx),
        g_app.image_rect.Y + static_cast<int>(py * sy),
        std::max(1, static_cast<int>(pw * sx)),
        std::max(1, static_cast<int>(ph * sy)));
}

Rect part_to_source_rect(const Part& part) {
    return Rect(
        std::max(0, static_cast<int>(std::round(part.x * g_app.part_scale_x))),
        std::max(0, static_cast<int>(std::round(part.y * g_app.part_scale_y))),
        std::max(1, static_cast<int>(std::round(part.w * g_app.part_scale_x))),
        std::max(1, static_cast<int>(std::round(part.h * g_app.part_scale_y))));
}

Rect image_rect_to_screen_rect(int x, int y, int w, int h) {
    if (!g_app.image || g_app.image_rect.Width <= 0 || g_app.image_rect.Height <= 0) {
        return canvas_rect(g_main);
    }
    double sx = static_cast<double>(g_app.image_rect.Width) / g_app.image->GetWidth();
    double sy = static_cast<double>(g_app.image_rect.Height) / g_app.image->GetHeight();
    int left = g_app.image_rect.X + static_cast<int>(std::floor(x * sx));
    int top = g_app.image_rect.Y + static_cast<int>(std::floor(y * sy));
    int right = g_app.image_rect.X + static_cast<int>(std::ceil((x + w) * sx));
    int bottom = g_app.image_rect.Y + static_cast<int>(std::ceil((y + h) * sy));
    return Rect(left, top, std::max(1, right - left), std::max(1, bottom - top));
}

void set_tool(Tool tool) {
    g_app.tool = tool;
    SendMessageW(g_select_tool, BM_SETCHECK, tool == Tool::Select ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(g_pencil_tool, BM_SETCHECK, tool == Tool::Pencil ? BST_CHECKED : BST_UNCHECKED, 0);
    SendMessageW(g_eraser_tool, BM_SETCHECK, tool == Tool::Eraser ? BST_CHECKED : BST_UNCHECKED, 0);
    invalidate_tool_ui();
    set_status(std::wstring(L"Ferramenta ativa: ") + tool_name(tool));
}

void update_zoom_label() {
    if (!g_zoom_label) {
        return;
    }
    std::wstring label = g_app.fit_to_view ? L"Fit" : std::to_wstring(static_cast<int>(g_app.zoom * 100.0)) + L"%";
    SetWindowTextW(g_zoom_label, label.c_str());
}

void set_zoom(double zoom) {
    g_app.fit_to_view = false;
    g_app.zoom = std::max(0.25, std::min(16.0, zoom));
    update_zoom_label();
    InvalidateRect(g_main, nullptr, FALSE);
}

void set_zoom_at(double zoom, int screen_x, int screen_y) {
    if (!g_app.image || g_app.image_rect.Width <= 0 || g_app.image_rect.Height <= 0) {
        set_zoom(zoom);
        return;
    }

    POINT image_point = screen_to_image_point(screen_x, screen_y);
    if (!image_point_inside(image_point)) {
        set_zoom(zoom);
        return;
    }
    g_app.fit_to_view = false;
    g_app.zoom = std::max(0.25, std::min(16.0, zoom));

    Rect canvas = canvas_rect(g_main);
    int scaled_w = static_cast<int>(g_app.image->GetWidth() * g_app.zoom);
    int scaled_h = static_cast<int>(g_app.image->GetHeight() * g_app.zoom);
    g_app.pan_x = static_cast<int>(std::round(screen_x - image_point.x * g_app.zoom - canvas.X - (canvas.Width - scaled_w) / 2.0));
    g_app.pan_y = static_cast<int>(std::round(screen_y - image_point.y * g_app.zoom - canvas.Y - (canvas.Height - scaled_h) / 2.0));
    update_zoom_label();
    InvalidateRect(g_main, nullptr, FALSE);
}

void fit_zoom() {
    g_app.fit_to_view = true;
    g_app.pan_x = 0;
    g_app.pan_y = 0;
    update_zoom_label();
    InvalidateRect(g_main, nullptr, FALSE);
}

void fill_brush_stamp(Graphics& image_g, SolidBrush& brush, int center_x, int center_y, int size) {
    int left = center_x - size / 2;
    int top = center_y - size / 2;
    if (g_app.brush_shape == BrushShape::Square) {
        image_g.FillRectangle(&brush, left, top, size, size);
    } else {
        image_g.FillEllipse(&brush, left, top, size, size);
    }
}

void paint_at(int screen_x, int screen_y) {
    POINT p = screen_to_image_point(screen_x, screen_y);
    if (!image_point_inside(p)) {
        return;
    }
    g_app.brush_size = clamp_int(get_control_int(g_brush_size, g_app.brush_size), 1, 128);
    Graphics image_g(g_app.image.get());
    image_g.SetSmoothingMode(g_app.brush_shape == BrushShape::Round ? Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
    image_g.SetCompositingMode(g_app.tool == Tool::Eraser ? CompositingModeSourceCopy : Gdiplus::CompositingModeSourceOver);
    Color color = g_app.tool == Tool::Eraser
        ? Color(0, 0, 0, 0)
        : Color(255, GetRValue(g_app.brush_color), GetGValue(g_app.brush_color), GetBValue(g_app.brush_color));
    SolidBrush brush(color);
    int radius = std::max(1, g_app.brush_size);
    fill_brush_stamp(image_g, brush, p.x, p.y, radius);

    Rect dirty = image_rect_to_screen_rect(p.x - radius, p.y - radius, radius * 2, radius * 2);
    RECT dirty_win = inflated_win_rect(dirty, 4);
    InvalidateRect(g_main, &dirty_win, FALSE);
    if (g_app.preview_rect.Width > 0 && g_app.preview_rect.Height > 0) {
        RECT preview = inflated_win_rect(g_app.preview_rect, 4);
        InvalidateRect(g_main, &preview, FALSE);
    }
}

void paint_stroke(int from_x, int from_y, int to_x, int to_y, Tool tool) {
    POINT start = screen_to_image_point(from_x, from_y);
    POINT end = screen_to_image_point(to_x, to_y);
    if (!image_point_inside(start) && !image_point_inside(end)) {
        return;
    }

    g_app.brush_size = clamp_int(get_control_int(g_brush_size, g_app.brush_size), 1, 128);
    Graphics image_g(g_app.image.get());
    image_g.SetSmoothingMode(g_app.brush_shape == BrushShape::Round ? Gdiplus::SmoothingModeAntiAlias : Gdiplus::SmoothingModeNone);
    image_g.SetCompositingMode(tool == Tool::Eraser ? CompositingModeSourceCopy : Gdiplus::CompositingModeSourceOver);
    Color color = tool == Tool::Eraser
        ? Color(0, 0, 0, 0)
        : Color(255, GetRValue(g_app.brush_color), GetGValue(g_app.brush_color), GetBValue(g_app.brush_color));
    SolidBrush brush(color);

    int radius = std::max(1, g_app.brush_size);
    int dx = end.x - start.x;
    int dy = end.y - start.y;
    int steps = std::max(1, std::max(std::abs(dx), std::abs(dy)) / std::max(1, radius / 2));
    int dirty_left = INT_MAX;
    int dirty_top = INT_MAX;
    int dirty_right = INT_MIN;
    int dirty_bottom = INT_MIN;

    for (int i = 0; i <= steps; ++i) {
        int px = start.x + dx * i / steps;
        int py = start.y + dy * i / steps;
        POINT p{px, py};
        if (!image_point_inside(p)) {
            continue;
        }
        int left = px - radius / 2;
        int top = py - radius / 2;
        fill_brush_stamp(image_g, brush, px, py, radius);
        dirty_left = std::min(dirty_left, left);
        dirty_top = std::min(dirty_top, top);
        dirty_right = std::max(dirty_right, left + radius);
        dirty_bottom = std::max(dirty_bottom, top + radius);
    }

    if (dirty_left == INT_MAX) {
        return;
    }

    Rect dirty = image_rect_to_screen_rect(dirty_left - 2, dirty_top - 2, dirty_right - dirty_left + 4, dirty_bottom - dirty_top + 4);
    RECT dirty_win = inflated_win_rect(dirty, 8);
    InvalidateRect(g_main, &dirty_win, FALSE);
    if (g_app.preview_rect.Width > 0 && g_app.preview_rect.Height > 0) {
        RECT preview = inflated_win_rect(g_app.preview_rect, 4);
        InvalidateRect(g_main, &preview, FALSE);
    }
}

void draw_section_label(Graphics& g, const wchar_t* title, const wchar_t* subtitle, int x, int y, int w) {
    SolidBrush title_brush(Color(255, 244, 244, 244));
    SolidBrush subtitle_brush(Color(255, 166, 166, 170));
    SolidBrush dot(Color(255, 0, 158, 205));
    Gdiplus::FontFamily family(L"Segoe UI");
    Gdiplus::Font title_font(&family, 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font subtitle_font(&family, 9.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    g.FillEllipse(&dot, x, y + 5, 7, 7);
    g.DrawString(title, -1, &title_font, Gdiplus::PointF(static_cast<float>(x + 13), static_cast<float>(y)), &title_brush);
    if (subtitle && subtitle[0]) {
        g.DrawString(subtitle, -1, &subtitle_font, Gdiplus::PointF(static_cast<float>(x + 13), static_cast<float>(y + 15)), &subtitle_brush);
    }
    Pen line(Color(38, 255, 255, 255), 1.0f);
    g.DrawLine(&line, x, y + 31, x + w, y + 31);
}

void draw_panel_shell(Graphics& g, const Rect& rect, Color fill, Color border) {
    SolidBrush brush(fill);
    Pen pen(border, 1.0f);
    g.FillRectangle(&brush, rect);
    g.DrawRectangle(&pen, rect);

    Pen top_light(Color(36, 255, 255, 255), 1.0f);
    g.DrawLine(&top_light, rect.X + 1, rect.Y + 1, rect.X + rect.Width - 2, rect.Y + 1);
}

void draw_tool_options_panel(Graphics& g) {
    if (!g_app.tool_panel_open || g_app.tool_panel_rect.Width <= 0 || g_app.tool_panel_rect.Height <= 0) {
        return;
    }

    Rect panel = g_app.tool_panel_rect;
    SolidBrush shadow(Color(95, 0, 0, 0));
    g.FillRectangle(&shadow, panel.X + 8, panel.Y + 8, panel.Width, panel.Height);

    Gdiplus::GraphicsPath panel_path;
    int d = 16;
    panel_path.AddArc(panel.X, panel.Y, d, d, 180, 90);
    panel_path.AddArc(panel.X + panel.Width - d, panel.Y, d, d, 270, 90);
    panel_path.AddArc(panel.X + panel.Width - d, panel.Y + panel.Height - d, d, d, 0, 90);
    panel_path.AddArc(panel.X, panel.Y + panel.Height - d, d, d, 90, 90);
    panel_path.CloseFigure();
    SolidBrush panel_fill(Color(248, 34, 36, 40));
    Pen panel_border(Color(128, 120, 128, 142), 1.0f);
    g.FillPath(&panel_fill, &panel_path);
    g.DrawPath(&panel_border, &panel_path);

    Gdiplus::FontFamily family(L"Segoe UI");
    Gdiplus::Font title_font(&family, 14.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::Font label_font(&family, 12.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    SolidBrush title_brush(Color(255, 245, 247, 250));
    SolidBrush label_brush(Color(255, 215, 219, 226));
    SolidBrush muted(Color(255, 176, 182, 193));

    Rect preview(panel.X + 18, panel.Y + 18, 98, 98);
    SolidBrush preview_bg(Color(255, 50, 53, 58));
    g.FillRectangle(&preview_bg, preview);

    Pen guide(Color(255, 130, 134, 140), 1.0f);
    int cx = preview.X + preview.Width / 2;
    int cy = preview.Y + preview.Height / 2;
    g.DrawEllipse(&guide, preview.X + 9, preview.Y + 9, preview.Width - 18, preview.Height - 18);
    g.DrawLine(&guide, preview.X + 18, cy, preview.X + preview.Width - 18, cy);
    g.DrawLine(&guide, cx, preview.Y + 18, cx, preview.Y + preview.Height - 18);

    int preview_size = clamp_int(g_app.brush_size, 4, 34);
    SolidBrush stamp(g_app.tool == Tool::Eraser ? Color(255, 210, 214, 220) : Color(255, GetRValue(g_app.brush_color), GetGValue(g_app.brush_color), GetBValue(g_app.brush_color)));
    if (g_app.brush_shape == BrushShape::Square) {
        g.FillRectangle(&stamp, cx - preview_size / 2, cy - preview_size / 2, preview_size, preview_size);
    } else {
        g.FillEllipse(&stamp, cx - preview_size / 2, cy - preview_size / 2, preview_size, preview_size);
    }

    std::wstring title = std::wstring(g_app.tool == Tool::Eraser ? L"Borracha" : L"Lapis") + L" profissional";
    g.DrawString(title.c_str(), -1, &title_font, Gdiplus::PointF(static_cast<float>(panel.X + 132), static_cast<float>(panel.Y + 20)), &title_brush);
    g.DrawString(L"Tamanho:", -1, &label_font, Gdiplus::PointF(static_cast<float>(panel.X + 132), static_cast<float>(panel.Y + 54)), &label_brush);
    g.DrawString(L"Dureza:", -1, &label_font, Gdiplus::PointF(static_cast<float>(panel.X + 132), static_cast<float>(panel.Y + 100)), &label_brush);
    g.DrawString(L"Formato", -1, &label_font, Gdiplus::PointF(static_cast<float>(panel.X + 18), static_cast<float>(panel.Y + 136)), &muted);

    Pen slider(Color(255, 92, 96, 104), 2.0f);
    Pen active(Color(255, 0, 158, 205), 3.0f);
    int size_y = panel.Y + 72;
    int hard_y = panel.Y + 118;
    int slider_left = panel.X + 132;
    int slider_right = panel.X + 300;
    g.DrawLine(&slider, slider_left, size_y, slider_right, size_y);
    g.DrawLine(&slider, slider_left, hard_y, slider_right, hard_y);
    g.DrawLine(&active, slider_left, size_y, slider_left + clamp_int(g_app.brush_size, 1, 128) * 168 / 128, size_y);
    g.DrawLine(&active, slider_left, hard_y, slider_left + clamp_int(g_app.brush_hardness, 1, 100) * 168 / 100, hard_y);
}

void render_scene(HWND hwnd, Graphics& g, const RECT& client, const RECT& repaint) {
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    g.Clear(Color(255, 5, 5, 6));

    Pen background_grid(Color(10, 255, 255, 255), 1.0f);
    int bg_start_x = (repaint.left / 26) * 26;
    int bg_start_y = (repaint.top / 26) * 26;
    for (int x = bg_start_x; x < repaint.right; x += 26) {
        g.DrawLine(&background_grid, x, 0, x, client.bottom);
    }
    for (int y = bg_start_y; y < repaint.bottom; y += 26) {
        g.DrawLine(&background_grid, 0, y, client.right, y);
    }

    SolidBrush menu(Color(245, 6, 7, 9));
    g.FillRectangle(&menu, 0, 0, client.right, UI_MENU_H);

    SolidBrush options(Color(245, 18, 20, 25));
    g.FillRectangle(&options, 0, UI_MENU_H, client.right, UI_OPTIONS_H);
    Pen split(Color(46, 255, 255, 255), 1.0f);
    g.DrawLine(&split, 0, UI_MENU_H - 1, client.right, UI_MENU_H - 1);
    g.DrawLine(&split, 0, UI_MENU_H + UI_OPTIONS_H - 1, client.right, UI_MENU_H + UI_OPTIONS_H - 1);

    SolidBrush toolbar(Color(246, 16, 18, 22));
    g.FillRectangle(&toolbar, 0, UI_MENU_H + UI_OPTIONS_H, UI_TOOLBAR_W, client.bottom);
    Pen toolbar_line(Color(44, 255, 255, 255), 1.0f);
    g.DrawLine(&toolbar_line, UI_TOOLBAR_W - 1, UI_MENU_H + UI_OPTIONS_H, UI_TOOLBAR_W - 1, client.bottom - UI_STATUS_H);

    Rect left_panel(UI_TOOLBAR_W, UI_MENU_H + UI_OPTIONS_H, UI_LEFT_PANEL_W, client.bottom - UI_MENU_H - UI_OPTIONS_H - UI_STATUS_H);
    Rect right_panel(client.right - UI_RIGHT_PANEL_W, UI_MENU_H + UI_OPTIONS_H, UI_RIGHT_PANEL_W, client.bottom - UI_MENU_H - UI_OPTIONS_H - UI_STATUS_H);
    draw_panel_shell(g, left_panel, Color(235, 12, 15, 21), Color(54, 255, 255, 255));
    draw_panel_shell(g, right_panel, Color(235, 12, 15, 21), Color(54, 255, 255, 255));

    SolidBrush brand(Color(255, 244, 244, 244));
    SolidBrush muted(Color(255, 166, 166, 170));
    Gdiplus::FontFamily brand_family(L"Segoe UI");
    Gdiplus::Font brand_font(&brand_family, 14.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font small_font(&brand_family, 10.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    g.DrawString(L"LexTeeworlds", -1, &brand_font, Gdiplus::PointF(470.0f, 9.0f), &brand);
    g.DrawString(L"Texture IDE 0.1", -1, &small_font, Gdiplus::PointF(570.0f, 11.0f), &muted);
    Pen brand_line(Color(255, 0, 158, 205), 2.0f);
    g.DrawLine(&brand_line, 470, 29, 558, 29);
    draw_section_label(g, L"Templates", L"Texturas oficiais", UI_TOOLBAR_W + 16, UI_MENU_H + UI_OPTIONS_H + 8, UI_LEFT_PANEL_W - 32);
    draw_section_label(g, L"Partes", L"Areas editaveis", UI_TOOLBAR_W + 16, UI_MENU_H + UI_OPTIONS_H + 185, UI_LEFT_PANEL_W - 32);
    draw_section_label(g, g_app.dev_mode ? L"Modo-dev" : L"Ferramenta", g_app.dev_mode ? L"IDs, JSON e preview" : L"Preview e atalhos", client.right - UI_RIGHT_PANEL_W + 16, UI_MENU_H + UI_OPTIONS_H + 8, UI_RIGHT_PANEL_W - 32);

    Rect agent(client.right - UI_RIGHT_PANEL_W + 16, client.bottom - UI_STATUS_H - 104, UI_RIGHT_PANEL_W - 32, 82);
    Gdiplus::GraphicsPath agent_path;
    int ad = 12;
    agent_path.AddArc(agent.X, agent.Y, ad, ad, 180, 90);
    agent_path.AddArc(agent.X + agent.Width - ad, agent.Y, ad, ad, 270, 90);
    agent_path.AddArc(agent.X + agent.Width - ad, agent.Y + agent.Height - ad, ad, ad, 0, 90);
    agent_path.AddArc(agent.X, agent.Y + agent.Height - ad, ad, ad, 90, 90);
    agent_path.CloseFigure();
    SolidBrush agent_fill(Color(145, 18, 22, 30));
    Pen agent_border(Color(58, 0, 158, 205), 1.0f);
    g.FillPath(&agent_fill, &agent_path);
    g.DrawPath(&agent_border, &agent_path);
    Gdiplus::Font agent_title(&brand_family, 12.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::Font agent_text_font(&brand_family, 10.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    g.DrawString(L"Agente IA", -1, &agent_title, Gdiplus::PointF(static_cast<float>(agent.X + 12), static_cast<float>(agent.Y + 12)), &brand);
    Gdiplus::StringFormat agent_format;
    g.DrawString(L"Cantinho reservado para o assistente do editor de foto.", -1, &agent_text_font, Gdiplus::RectF(static_cast<float>(agent.X + 12), static_cast<float>(agent.Y + 34), static_cast<float>(agent.Width - 24), 38.0f), &agent_format, &muted);

    Rect canvas = canvas_rect(hwnd);
    SolidBrush canvas_brush(Color(255, 9, 11, 15));
    g.FillRectangle(&canvas_brush, canvas);

    Pen canvas_grid(Color(26, 255, 255, 255), 1.0f);
    constexpr int canvas_grid_step = 32;
    int canvas_start_x = canvas.X + std::max(1, static_cast<int>((repaint.left - canvas.X) / canvas_grid_step)) * canvas_grid_step;
    int canvas_start_y = canvas.Y + std::max(1, static_cast<int>((repaint.top - canvas.Y) / canvas_grid_step)) * canvas_grid_step;
    int canvas_end_x = std::min(canvas.X + canvas.Width, static_cast<int>(repaint.right));
    int canvas_end_y = std::min(canvas.Y + canvas.Height, static_cast<int>(repaint.bottom));
    for (int x = canvas_start_x; x < canvas_end_x; x += canvas_grid_step) {
        g.DrawLine(&canvas_grid, x, canvas.Y, x, canvas.Y + canvas.Height);
    }
    for (int y = canvas_start_y; y < canvas_end_y; y += canvas_grid_step) {
        g.DrawLine(&canvas_grid, canvas.X, y, canvas.X + canvas.Width, y);
    }

    Pen frame(Color(210, 0, 158, 205), 2.0f);
    g.DrawRectangle(&frame, canvas);

    if (g_app.image) {
        g_app.image_rect = fit_image_rect(canvas, g_app.image.get());
        draw_image_checker(g);
        g.DrawImage(g_app.image.get(), g_app.image_rect);
        draw_pixel_grid(g);

        Part* part = current_part();
        if (part) {
            Rect highlight = part_to_screen(*part);
            if (!g_app.show_all_parts) {
                SolidBrush dim(Color(145, 0, 0, 0));
                Gdiplus::Region dim_region(g_app.image_rect);
                dim_region.Exclude(highlight);
                g.FillRegion(&dim, &dim_region);
            }

            Pen cyan(Color(255, 85, 220, 255), 3.0f);
            g.DrawRectangle(&cyan, highlight);

            int max_preview_w = std::max(96, canvas.Width / 3);
            int max_preview_h = std::max(96, canvas.Height / 3);
            Rect src = part_to_source_rect(*part);
            int zoom = std::max(1, std::min({8, max_preview_w / std::max(1, src.Width), max_preview_h / std::max(1, src.Height)}));
            int preview_w = std::max(1, src.Width * zoom);
            int preview_h = std::max(1, src.Height * zoom);
            Rect preview(canvas.X + canvas.Width - preview_w - 18, canvas.Y + canvas.Height - preview_h - 18, preview_w, preview_h);
            g_app.preview_rect = preview;
            SolidBrush preview_bg(Color(230, 20, 25, 33));
            g.FillRectangle(&preview_bg, preview);
            g.DrawRectangle(&cyan, preview);
            g.DrawImage(g_app.image.get(), preview, src.X, src.Y, src.Width, src.Height, Gdiplus::UnitPixel);
        } else {
            g_app.preview_rect = Rect();
        }
    } else {
        g_app.preview_rect = Rect();
    }

    bool show_all_parts = g_app.show_all_parts || (g_show_all && SendMessageW(g_show_all, BM_GETCHECK, 0, 0) == BST_CHECKED);
    if (g_app.image && show_all_parts) {
        TextureTemplate* texture = current_template();
        if (texture) {
            Pen overlay_pen(Color(255, 0, 190, 240), 2.5f);
            SolidBrush overlay_fill(Color(42, 0, 158, 205));
            SolidBrush label_bg(Color(232, 5, 5, 6));
            SolidBrush label_text(Color(255, 244, 244, 244));
            Gdiplus::FontFamily overlay_family(L"Segoe UI");
            Gdiplus::Font overlay_font(&overlay_family, 10.0f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
            for (const Part& each : texture->parts) {
                Rect part_rect = part_to_screen(each);
                g.FillRectangle(&overlay_fill, part_rect);
                g.DrawRectangle(&overlay_pen, part_rect);
                std::wstring name = widen(each.id);
                Rect label_rect(part_rect.X, std::max(g_app.image_rect.Y, part_rect.Y - 20), std::max(58, static_cast<int>(name.size()) * 8 + 12), 18);
                g.FillRectangle(&label_bg, label_rect);
                g.DrawString(name.c_str(), -1, &overlay_font, Gdiplus::PointF(static_cast<float>(label_rect.X + 5), static_cast<float>(label_rect.Y + 5)), &label_text);
            }
        }
    }

    if (g_app.image && g_app.hover_canvas && g_app.tool != Tool::Select) {
        POINT p = screen_to_image_point(g_app.hover_mouse.x, g_app.hover_mouse.y);
        if (image_point_inside(p)) {
            double sx = static_cast<double>(g_app.image_rect.Width) / g_app.image->GetWidth();
            double sy = static_cast<double>(g_app.image_rect.Height) / g_app.image->GetHeight();
            int size = std::max(4, static_cast<int>(std::round(g_app.brush_size * std::max(sx, sy))));
            int left = g_app.hover_mouse.x - size / 2;
            int top = g_app.hover_mouse.y - size / 2;
            Pen outline(g_app.tool == Tool::Eraser ? Color(240, 255, 120, 120) : Color(240, 255, 255, 255), 1.5f);
            SolidBrush fill(g_app.tool == Tool::Eraser ? Color(55, 255, 80, 80) : Color(55, GetRValue(g_app.brush_color), GetGValue(g_app.brush_color), GetBValue(g_app.brush_color)));
            if (g_app.brush_shape == BrushShape::Square) {
                g.FillRectangle(&fill, left, top, size, size);
                g.DrawRectangle(&outline, left, top, size, size);
            } else {
                g.FillEllipse(&fill, left, top, size, size);
                g.DrawEllipse(&outline, left, top, size, size);
            }
        }
    }

    SolidBrush hud_bg(Color(215, 16, 20, 27));
    SolidBrush hud_text(Color(255, 226, 232, 240));
    Rect hud(canvas.X + 14, canvas.Y + canvas.Height - 34, 360, 24);
    g.FillRectangle(&hud_bg, hud);
    std::wstring hud_label = L"Zoom: " + (g_app.fit_to_view ? std::wstring(L"Fit") : std::to_wstring(static_cast<int>(g_app.zoom * 100.0)) + L"%");
    if (g_app.image && g_app.hover_canvas) {
        POINT p = screen_to_image_point(g_app.hover_mouse.x, g_app.hover_mouse.y);
        if (image_point_inside(p)) {
            hud_label += L"   Pixel: " + std::to_wstring(p.x) + L", " + std::to_wstring(p.y);
        }
    }
    Gdiplus::FontFamily family(L"Segoe UI");
    Gdiplus::Font font(&family, 10.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    g.DrawString(hud_label.c_str(), -1, &font, Gdiplus::PointF(static_cast<float>(hud.X + 8), static_cast<float>(hud.Y + 6)), &hud_text);

    draw_tool_options_panel(g);
}

void paint(HWND hwnd) {
    PAINTSTRUCT ps{};
    HDC hdc = BeginPaint(hwnd, &ps);
    RECT client{};
    GetClientRect(hwnd, &client);

    int paint_w = std::max(1, static_cast<int>(ps.rcPaint.right - ps.rcPaint.left));
    int paint_h = std::max(1, static_cast<int>(ps.rcPaint.bottom - ps.rcPaint.top));
    HDC memory_dc = CreateCompatibleDC(hdc);
    HBITMAP memory_bitmap = CreateCompatibleBitmap(hdc, paint_w, paint_h);
    HGDIOBJ old_bitmap = SelectObject(memory_dc, memory_bitmap);
    SetViewportOrgEx(memory_dc, -ps.rcPaint.left, -ps.rcPaint.top, nullptr);

    Graphics buffered(memory_dc);
    render_scene(hwnd, buffered, client, ps.rcPaint);
    buffered.Flush();

    BitBlt(hdc, ps.rcPaint.left, ps.rcPaint.top, paint_w, paint_h, memory_dc, 0, 0, SRCCOPY);
    SelectObject(memory_dc, old_bitmap);
    DeleteObject(memory_bitmap);
    DeleteDC(memory_dc);
    EndPaint(hwnd, &ps);
}

void select_part_from_point(int x, int y) {
    TextureTemplate* texture = current_template();
    if (!texture || !g_app.image) {
        return;
    }
    if (x < g_app.image_rect.X || y < g_app.image_rect.Y || x > g_app.image_rect.X + g_app.image_rect.Width || y > g_app.image_rect.Y + g_app.image_rect.Height) {
        return;
    }
    double sx = static_cast<double>(g_app.image->GetWidth()) / g_app.image_rect.Width;
    double sy = static_cast<double>(g_app.image->GetHeight()) / g_app.image_rect.Height;
    int ix = static_cast<int>((x - g_app.image_rect.X) * sx);
    int iy = static_cast<int>((y - g_app.image_rect.Y) * sy);
    for (size_t i = 0; i < texture->parts.size(); ++i) {
        const Part& part = texture->parts[i];
        Rect src = part_to_source_rect(part);
        if (ix >= src.X && ix <= src.X + src.Width && iy >= src.Y && iy <= src.Y + src.Height) {
            g_app.selected_part = static_cast<int>(i);
            SendMessageW(g_parts, LB_SETCURSEL, g_app.selected_part, 0);
            populate_dev_fields();
            InvalidateRect(g_main, nullptr, FALSE);
            return;
        }
    }
}

HWND make_child(HWND parent, const wchar_t* cls, const wchar_t* text, DWORD style, int id) {
    HWND child = CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, parent, reinterpret_cast<HMENU>(id), GetModuleHandleW(nullptr), nullptr);
    if (g_ui_font) {
        SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(g_ui_font), TRUE);
    }
    return child;
}

HWND make_button(HWND parent, const wchar_t* text, DWORD style, int id) {
    return make_child(parent, L"BUTTON", text, style | BS_OWNERDRAW, id);
}

void invalidate_tool_ui() {
    HWND buttons[] = {g_select_tool, g_pencil_tool, g_eraser_tool, g_color_pick, g_tool_preview, g_shape_round, g_shape_square};
    for (HWND hwnd : buttons) {
        if (hwnd) {
            InvalidateRect(hwnd, nullptr, TRUE);
        }
    }
}

bool is_checked_button(int id) {
    if (id == ID_SHAPE_ROUND || id == ID_SHAPE_SQUARE) {
        HWND button = id == ID_SHAPE_ROUND ? g_shape_round : g_shape_square;
        return button && SendMessageW(button, BM_GETCHECK, 0, 0) == BST_CHECKED;
    }
    if (id == ID_DEV || id == ID_SHOW_ALL || id == ID_TOOL_SELECT || id == ID_TOOL_PENCIL || id == ID_TOOL_ERASER) {
        HWND button = GetDlgItem(g_main, id);
        return button && SendMessageW(button, BM_GETCHECK, 0, 0) == BST_CHECKED;
    }
    return false;
}

void draw_tool_icon(HDC dc, int id, RECT r, COLORREF color) {
    HPEN pen = CreatePen(PS_SOLID, 2, color);
    HGDIOBJ old_pen = SelectObject(dc, pen);
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ old_brush = SelectObject(dc, brush);
    int cx = r.left + 15;
    int cy = r.top + (r.bottom - r.top) / 2;
    if (id == ID_TOOL_SELECT) {
        POINT pts[] = {{cx - 4, cy - 8}, {cx + 8, cy}, {cx + 1, cy + 2}, {cx + 5, cy + 10}, {cx + 1, cy + 11}, {cx - 3, cy + 4}, {cx - 8, cy + 9}};
        Polygon(dc, pts, 7);
    } else if (id == ID_TOOL_PENCIL) {
        MoveToEx(dc, cx - 7, cy + 8, nullptr);
        LineTo(dc, cx + 8, cy - 7);
        MoveToEx(dc, cx + 4, cy - 9, nullptr);
        LineTo(dc, cx + 10, cy - 3);
    } else if (id == ID_TOOL_ERASER) {
        RoundRect(dc, cx - 9, cy - 6, cx + 9, cy + 7, 4, 4);
        MoveToEx(dc, cx - 2, cy - 7, nullptr);
        LineTo(dc, cx + 8, cy + 5);
    } else if (id == ID_ZOOM_IN || id == ID_ZOOM_OUT) {
        Ellipse(dc, cx - 7, cy - 7, cx + 5, cy + 5);
        MoveToEx(dc, cx + 4, cy + 4, nullptr);
        LineTo(dc, cx + 11, cy + 11);
        MoveToEx(dc, cx - 4, cy - 1, nullptr);
        LineTo(dc, cx + 2, cy - 1);
        if (id == ID_ZOOM_IN) {
            MoveToEx(dc, cx - 1, cy - 4, nullptr);
            LineTo(dc, cx - 1, cy + 2);
        }
    }
    SelectObject(dc, old_pen);
    SelectObject(dc, old_brush);
    DeleteObject(pen);
    DeleteObject(brush);
}

void draw_owner_button(const DRAWITEMSTRUCT* item) {
    if (item->CtlType == ODT_LISTBOX) {
        RECT r = item->rcItem;
        bool selected = (item->itemState & ODS_SELECTED) != 0;
        bool odd = (item->itemID != static_cast<UINT>(-1)) && (item->itemID % 2 == 1);
        HBRUSH bg = CreateSolidBrush(selected ? RGB(0, 158, 205) : (odd ? RGB(15, 18, 24) : RGB(18, 21, 28)));
        FillRect(item->hDC, &r, bg);
        DeleteObject(bg);
        if (selected) {
            RECT accent{r.left, r.top, r.left + 4, r.bottom};
            HBRUSH accent_brush = CreateSolidBrush(RGB(244, 244, 244));
            FillRect(item->hDC, &accent, accent_brush);
            DeleteObject(accent_brush);
        }
        if (item->itemID != static_cast<UINT>(-1)) {
            wchar_t text[512] = {};
            SendMessageW(item->hwndItem, LB_GETTEXT, item->itemID, reinterpret_cast<LPARAM>(text));
            SetBkMode(item->hDC, TRANSPARENT);
            SetTextColor(item->hDC, selected ? RGB(255, 255, 255) : RGB(232, 236, 242));
            if (g_ui_font) {
                SelectObject(item->hDC, g_ui_font);
            }
            if (item->hwndItem == g_parts && g_app.image && item->itemID < static_cast<UINT>(current_template() ? current_template()->parts.size() : 0)) {
                const Part& part = current_template()->parts[item->itemID];
                Rect src = part_to_source_rect(part);
                RECT thumb{r.left + 6, r.top + 4, r.left + 34, r.bottom - 4};
                HBRUSH thumb_bg = CreateSolidBrush(RGB(12, 14, 18));
                FillRect(item->hDC, &thumb, thumb_bg);
                DeleteObject(thumb_bg);
                Graphics thumb_g(item->hDC);
                thumb_g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
                int thumb_w = std::max(1, static_cast<int>(thumb.right - thumb.left - 4));
                int thumb_h = std::max(1, static_cast<int>(thumb.bottom - thumb.top - 4));
                Rect dest(thumb.left + 2, thumb.top + 2, thumb_w, thumb_h);
                thumb_g.DrawImage(g_app.image.get(), dest, src.X, src.Y, src.Width, src.Height, Gdiplus::UnitPixel);
                HPEN thumb_border = CreatePen(PS_SOLID, 1, selected ? RGB(255, 255, 255) : RGB(80, 92, 110));
                HGDIOBJ old_pen = SelectObject(item->hDC, thumb_border);
                HGDIOBJ old_brush = SelectObject(item->hDC, GetStockObject(NULL_BRUSH));
                Rectangle(item->hDC, thumb.left, thumb.top, thumb.right, thumb.bottom);
                SelectObject(item->hDC, old_brush);
                SelectObject(item->hDC, old_pen);
                DeleteObject(thumb_border);
                r.left += 42;
            } else {
                r.left += 8;
            }
            DrawTextW(item->hDC, text, -1, &r, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
        HPEN separator = CreatePen(PS_SOLID, 1, RGB(30, 34, 43));
        HGDIOBJ old_pen = SelectObject(item->hDC, separator);
        MoveToEx(item->hDC, item->rcItem.left, item->rcItem.bottom - 1, nullptr);
        LineTo(item->hDC, item->rcItem.right, item->rcItem.bottom - 1);
        SelectObject(item->hDC, old_pen);
        DeleteObject(separator);
        return;
    }

    RECT r = item->rcItem;
    bool pressed = (item->itemState & ODS_SELECTED) != 0;
    bool checked = is_checked_button(static_cast<int>(item->CtlID));
    bool disabled = (item->itemState & ODS_DISABLED) != 0;

    COLORREF bg = checked ? RGB(0, 158, 205) : (pressed ? RGB(56, 62, 72) : RGB(20, 24, 32));
    COLORREF border = checked ? RGB(125, 216, 209) : (pressed ? RGB(154, 164, 180) : RGB(82, 92, 108));
    COLORREF text = disabled ? RGB(120, 128, 140) : RGB(235, 241, 248);

    HBRUSH bg_brush = CreateSolidBrush(bg);
    HPEN border_pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ old_brush = SelectObject(item->hDC, bg_brush);
    HGDIOBJ old_pen = SelectObject(item->hDC, border_pen);
    RoundRect(item->hDC, r.left, r.top, r.right, r.bottom, 7, 7);
    if (checked) {
        RECT glow{r.left + 2, r.top + 2, r.right - 2, r.top + 5};
        HBRUSH glow_brush = CreateSolidBrush(RGB(122, 226, 255));
        FillRect(item->hDC, &glow, glow_brush);
        DeleteObject(glow_brush);
        RECT active_bar{r.left + 2, r.top + 7, r.left + 5, r.bottom - 7};
        HBRUSH active_brush = CreateSolidBrush(RGB(244, 244, 244));
        FillRect(item->hDC, &active_bar, active_brush);
        DeleteObject(active_brush);
    } else if (pressed) {
        RECT press_shadow{r.left + 1, r.top + 1, r.right - 1, r.bottom - 1};
        HPEN press_pen = CreatePen(PS_SOLID, 1, RGB(25, 28, 34));
        HGDIOBJ press_old_pen = SelectObject(item->hDC, press_pen);
        HGDIOBJ press_old_brush = SelectObject(item->hDC, GetStockObject(NULL_BRUSH));
        RoundRect(item->hDC, press_shadow.left, press_shadow.top, press_shadow.right, press_shadow.bottom, 6, 6);
        SelectObject(item->hDC, press_old_brush);
        SelectObject(item->hDC, press_old_pen);
        DeleteObject(press_pen);
    }
    SelectObject(item->hDC, old_pen);
    SelectObject(item->hDC, old_brush);
    DeleteObject(border_pen);
    DeleteObject(bg_brush);

    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, text);
    if (g_ui_font) {
        SelectObject(item->hDC, g_ui_font);
    }

    int id = static_cast<int>(item->CtlID);
    bool has_icon = id == ID_TOOL_SELECT || id == ID_TOOL_PENCIL || id == ID_TOOL_ERASER || id == ID_ZOOM_IN || id == ID_ZOOM_OUT;
    if (has_icon) {
        draw_tool_icon(item->hDC, id, r, text);
        if ((r.right - r.left) <= 48 && (id == ID_TOOL_SELECT || id == ID_TOOL_PENCIL || id == ID_TOOL_ERASER)) {
            return;
        }
        r.left += 28;
    } else if (id == ID_COLOR_PICK) {
        RECT swatch{r.left + 10, r.top + 8, r.left + 28, r.bottom - 8};
        HBRUSH color_brush = CreateSolidBrush(g_app.brush_color);
        FillRect(item->hDC, &swatch, color_brush);
        DeleteObject(color_brush);
        HPEN swatch_border = CreatePen(PS_SOLID, 1, RGB(235, 241, 248));
        HGDIOBJ old_pen = SelectObject(item->hDC, swatch_border);
        HGDIOBJ old_brush = SelectObject(item->hDC, GetStockObject(NULL_BRUSH));
        Rectangle(item->hDC, swatch.left, swatch.top, swatch.right, swatch.bottom);
        SelectObject(item->hDC, old_brush);
        SelectObject(item->hDC, old_pen);
        DeleteObject(swatch_border);
        r.left += 26;
    }

    wchar_t label[128] = {};
    GetWindowTextW(item->hwndItem, label, 128);
    if (checked) {
        r.left += 5;
    }
    DrawTextW(item->hDC, label, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
}

void draw_tool_preview(const DRAWITEMSTRUCT* item) {
    RECT r = item->rcItem;
    HBRUSH bg = CreateSolidBrush(RGB(13, 16, 22));
    FillRect(item->hDC, &r, bg);
    DeleteObject(bg);

    HPEN border = CreatePen(PS_SOLID, 1, RGB(82, 92, 108));
    HGDIOBJ old_pen = SelectObject(item->hDC, border);
    HBRUSH old_brush = reinterpret_cast<HBRUSH>(SelectObject(item->hDC, GetStockObject(NULL_BRUSH)));
    RoundRect(item->hDC, r.left, r.top, r.right, r.bottom, 8, 8);
    SelectObject(item->hDC, old_brush);
    SelectObject(item->hDC, old_pen);
    DeleteObject(border);

    RECT preview_grid{r.left + 12, r.top + 52, r.right - 12, r.bottom - 10};
    HBRUSH grid_bg = CreateSolidBrush(RGB(18, 22, 30));
    FillRect(item->hDC, &preview_grid, grid_bg);
    DeleteObject(grid_bg);
    HPEN grid_pen = CreatePen(PS_SOLID, 1, RGB(32, 38, 48));
    HGDIOBJ grid_old_pen = SelectObject(item->hDC, grid_pen);
    for (int gx = preview_grid.left; gx < preview_grid.right; gx += 12) {
        MoveToEx(item->hDC, gx, preview_grid.top, nullptr);
        LineTo(item->hDC, gx, preview_grid.bottom);
    }
    for (int gy = preview_grid.top; gy < preview_grid.bottom; gy += 12) {
        MoveToEx(item->hDC, preview_grid.left, gy, nullptr);
        LineTo(item->hDC, preview_grid.right, gy);
    }
    SelectObject(item->hDC, grid_old_pen);
    DeleteObject(grid_pen);

    SetBkMode(item->hDC, TRANSPARENT);
    SetTextColor(item->hDC, RGB(235, 241, 248));
    if (g_ui_font) {
        SelectObject(item->hDC, g_ui_font);
    }
    RECT title = r;
    title.left += 12;
    title.top += 8;
    title.bottom = title.top + 24;
    std::wstring text = std::wstring(L"Ativo: ") + tool_name(g_app.tool);
    DrawTextW(item->hDC, text.c_str(), -1, &title, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT meta = r;
    meta.left += 12;
    meta.top += 30;
    meta.bottom = meta.top + 18;
    SetTextColor(item->hDC, RGB(166, 166, 170));
    std::wstring brush_meta = L"Brush " + std::to_wstring(g_app.brush_size) + L"px  " + brush_shape_name(g_app.brush_shape);
    DrawTextW(item->hDC, brush_meta.c_str(), -1, &meta, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT color_swatch{r.right - 40, r.top + 12, r.right - 16, r.top + 36};
    HBRUSH swatch = CreateSolidBrush(g_app.tool == Tool::Eraser ? RGB(30, 34, 43) : g_app.brush_color);
    FillRect(item->hDC, &color_swatch, swatch);
    DeleteObject(swatch);
    HPEN swatch_pen = CreatePen(PS_SOLID, 1, RGB(125, 216, 209));
    HGDIOBJ swatch_old_pen = SelectObject(item->hDC, swatch_pen);
    HGDIOBJ swatch_old_brush = SelectObject(item->hDC, GetStockObject(NULL_BRUSH));
    Rectangle(item->hDC, color_swatch.left, color_swatch.top, color_swatch.right, color_swatch.bottom);
    SelectObject(item->hDC, swatch_old_brush);
    SelectObject(item->hDC, swatch_old_pen);
    DeleteObject(swatch_pen);

    int cx = r.left + 46;
    int cy = r.top + 84;
    int radius = clamp_int(g_app.brush_size, 1, 128);
    int preview_radius = clamp_int(radius, 4, 28);
    HPEN cyan = CreatePen(PS_SOLID, 2, RGB(91, 220, 255));
    HGDIOBJ old = SelectObject(item->hDC, cyan);
    if (g_app.tool == Tool::Eraser) {
        HBRUSH eraser = CreateSolidBrush(RGB(25, 32, 42));
        HGDIOBJ old_b = SelectObject(item->hDC, eraser);
        if (g_app.brush_shape == BrushShape::Square) {
            Rectangle(item->hDC, cx - preview_radius, cy - preview_radius, cx + preview_radius, cy + preview_radius);
        } else {
            Ellipse(item->hDC, cx - preview_radius, cy - preview_radius, cx + preview_radius, cy + preview_radius);
        }
        SelectObject(item->hDC, old_b);
        DeleteObject(eraser);
    } else if (g_app.tool == Tool::Pencil) {
        HBRUSH brush = CreateSolidBrush(g_app.brush_color);
        HGDIOBJ old_b = SelectObject(item->hDC, brush);
        if (g_app.brush_shape == BrushShape::Square) {
            Rectangle(item->hDC, cx - preview_radius, cy - preview_radius, cx + preview_radius, cy + preview_radius);
        } else {
            Ellipse(item->hDC, cx - preview_radius, cy - preview_radius, cx + preview_radius, cy + preview_radius);
        }
        SelectObject(item->hDC, old_b);
        DeleteObject(brush);
    } else {
        MoveToEx(item->hDC, cx - 18, cy - 18, nullptr);
        LineTo(item->hDC, cx + 18, cy + 18);
        MoveToEx(item->hDC, cx + 18, cy - 18, nullptr);
        LineTo(item->hDC, cx - 18, cy + 18);
    }
    SelectObject(item->hDC, old);
    DeleteObject(cyan);

    RECT hint = r;
    hint.left += 88;
    hint.top += 62;
    hint.right -= 10;
    hint.bottom -= 8;
    SetTextColor(item->hDC, RGB(177, 186, 201));
    DrawTextW(item->hDC, L"Botao direito: opcoes\nCtrl+Z / Ctrl+Y\nCtrl + roda: zoom", -1, &hint, DT_LEFT | DT_WORDBREAK);
}

void sync_options_controls() {
    if (g_options_size) {
        set_control_int_if_changed(g_options_size, g_app.brush_size);
    }
    if (g_options_hardness) {
        set_control_int_if_changed(g_options_hardness, g_app.brush_hardness);
    }
    if (g_shape_round) {
        SendMessageW(g_shape_round, BM_SETCHECK, g_app.brush_shape == BrushShape::Round ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    if (g_shape_square) {
        SendMessageW(g_shape_square, BM_SETCHECK, g_app.brush_shape == BrushShape::Square ? BST_CHECKED : BST_UNCHECKED, 0);
    }
    invalidate_tool_ui();
}

void layout_tool_panel_controls(HWND hwnd) {
    HWND controls[] = {g_options_size, g_options_hardness, g_shape_round, g_shape_square, g_options_color, g_options_close};
    int show = g_app.tool_panel_open ? SW_SHOW : SW_HIDE;
    for (HWND control : controls) {
        if (control) {
            ShowWindow(control, show);
        }
    }
    if (!g_app.tool_panel_open || g_app.tool_panel_rect.Width <= 0) {
        return;
    }

    Rect panel = g_app.tool_panel_rect;
    MoveWindow(g_options_size, panel.X + 246, panel.Y + 36, 58, 26, TRUE);
    MoveWindow(g_options_hardness, panel.X + 246, panel.Y + 82, 58, 26, TRUE);
    MoveWindow(g_options_close, panel.X + panel.Width - 36, panel.Y + 10, 24, 24, TRUE);
    MoveWindow(g_shape_round, panel.X + 18, panel.Y + 158, 92, 30, TRUE);
    MoveWindow(g_shape_square, panel.X + 118, panel.Y + 158, 98, 30, TRUE);
    MoveWindow(g_options_color, panel.X + 226, panel.Y + 158, 78, 30, TRUE);
    sync_options_controls();
    InvalidateRect(hwnd, nullptr, FALSE);
}

void draw_options_panel(HWND hwnd, HDC dc) {
    RECT client{};
    GetClientRect(hwnd, &client);
    HBRUSH bg = CreateSolidBrush(RGB(34, 36, 40));
    FillRect(dc, &client, bg);
    DeleteObject(bg);

    HPEN border = CreatePen(PS_SOLID, 1, RGB(76, 80, 88));
    HGDIOBJ old_pen = SelectObject(dc, border);
    HGDIOBJ old_brush = SelectObject(dc, GetStockObject(NULL_BRUSH));
    RoundRect(dc, 0, 0, client.right, client.bottom, 8, 8);
    SelectObject(dc, old_brush);
    SelectObject(dc, old_pen);
    DeleteObject(border);

    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, RGB(238, 241, 245));
    if (g_ui_font) {
        SelectObject(dc, g_ui_font);
    }

    RECT title{132, 10, client.right - 12, 32};
    std::wstring title_text = std::wstring(g_app.tool == Tool::Eraser ? L"Borracha" : L"Lapis") + L" profissional";
    DrawTextW(dc, title_text.c_str(), -1, &title, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    RECT preview{18, 18, 116, 116};
    HBRUSH preview_bg = CreateSolidBrush(RGB(50, 53, 58));
    FillRect(dc, &preview, preview_bg);
    DeleteObject(preview_bg);

    HPEN guide = CreatePen(PS_SOLID, 1, RGB(130, 134, 140));
    HGDIOBJ guide_old = SelectObject(dc, guide);
    int cx = (preview.left + preview.right) / 2;
    int cy = (preview.top + preview.bottom) / 2;
    Ellipse(dc, preview.left + 9, preview.top + 9, preview.right - 9, preview.bottom - 9);
    MoveToEx(dc, preview.left + 18, cy, nullptr);
    LineTo(dc, preview.right - 18, cy);
    MoveToEx(dc, cx, preview.top + 18, nullptr);
    LineTo(dc, cx, preview.bottom - 18);
    SelectObject(dc, guide_old);
    DeleteObject(guide);

    int preview_size = clamp_int(g_app.brush_size, 4, 34);
    HBRUSH stamp = CreateSolidBrush(g_app.tool == Tool::Eraser ? RGB(210, 214, 220) : g_app.brush_color);
    HGDIOBJ stamp_old = SelectObject(dc, stamp);
    if (g_app.brush_shape == BrushShape::Square) {
        Rectangle(dc, cx - preview_size / 2, cy - preview_size / 2, cx + preview_size / 2, cy + preview_size / 2);
    } else {
        Ellipse(dc, cx - preview_size / 2, cy - preview_size / 2, cx + preview_size / 2, cy + preview_size / 2);
    }
    SelectObject(dc, stamp_old);
    DeleteObject(stamp);

    SetTextColor(dc, RGB(210, 214, 220));
    RECT size_label{132, 42, 210, 62};
    RECT hard_label{132, 88, 210, 108};
    DrawTextW(dc, L"Tamanho:", -1, &size_label, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    DrawTextW(dc, L"Dureza:", -1, &hard_label, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    HPEN slider = CreatePen(PS_SOLID, 2, RGB(92, 96, 104));
    HGDIOBJ slider_old = SelectObject(dc, slider);
    MoveToEx(dc, 132, 70, nullptr);
    LineTo(dc, 300, 70);
    MoveToEx(dc, 132, 116, nullptr);
    LineTo(dc, 300, 116);
    SelectObject(dc, slider_old);
    DeleteObject(slider);

    HPEN active = CreatePen(PS_SOLID, 3, RGB(0, 158, 205));
    HGDIOBJ active_old = SelectObject(dc, active);
    int size_pos = 132 + clamp_int(g_app.brush_size, 1, 128) * 168 / 128;
    int hard_pos = 132 + clamp_int(g_app.brush_hardness, 1, 100) * 168 / 100;
    MoveToEx(dc, 132, 70, nullptr);
    LineTo(dc, size_pos, 70);
    MoveToEx(dc, 132, 116, nullptr);
    LineTo(dc, hard_pos, 116);
    SelectObject(dc, active_old);
    DeleteObject(active);

    SetTextColor(dc, RGB(186, 192, 202));
    RECT shape_label{18, 130, 120, 150};
    DrawTextW(dc, L"Formato", -1, &shape_label, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
}

LRESULT CALLBACK options_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE: {
        g_options_size = CreateWindowExW(0, L"EDIT", std::to_wstring(g_app.brush_size).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 246, 34, 58, 26, hwnd, reinterpret_cast<HMENU>(ID_OPTIONS), GetModuleHandleW(nullptr), nullptr);
        g_options_hardness = CreateWindowExW(0, L"EDIT", std::to_wstring(g_app.brush_hardness).c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, 246, 80, 58, 26, hwnd, reinterpret_cast<HMENU>(ID_HARDNESS), GetModuleHandleW(nullptr), nullptr);
        g_shape_round = CreateWindowExW(0, L"BUTTON", L"Redondo", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_OWNERDRAW, 18, 154, 92, 30, hwnd, reinterpret_cast<HMENU>(ID_SHAPE_ROUND), GetModuleHandleW(nullptr), nullptr);
        g_shape_square = CreateWindowExW(0, L"BUTTON", L"Quadrado", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | BS_OWNERDRAW, 118, 154, 98, 30, hwnd, reinterpret_cast<HMENU>(ID_SHAPE_SQUARE), GetModuleHandleW(nullptr), nullptr);
        HWND color = CreateWindowExW(0, L"BUTTON", L"Cor", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW, 226, 154, 78, 30, hwnd, reinterpret_cast<HMENU>(ID_COLOR_PICK), GetModuleHandleW(nullptr), nullptr);
        HWND close = CreateWindowExW(0, L"BUTTON", L"Fechar", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW, 312, 154, 76, 30, hwnd, reinterpret_cast<HMENU>(ID_OPTIONS_CLOSE), GetModuleHandleW(nullptr), nullptr);
        if (g_ui_font) {
            SendMessageW(g_options_size, WM_SETFONT, reinterpret_cast<WPARAM>(g_ui_font), TRUE);
            SendMessageW(g_options_hardness, WM_SETFONT, reinterpret_cast<WPARAM>(g_ui_font), TRUE);
            SendMessageW(g_shape_round, WM_SETFONT, reinterpret_cast<WPARAM>(g_ui_font), TRUE);
            SendMessageW(g_shape_square, WM_SETFONT, reinterpret_cast<WPARAM>(g_ui_font), TRUE);
            SendMessageW(color, WM_SETFONT, reinterpret_cast<WPARAM>(g_ui_font), TRUE);
            SendMessageW(close, WM_SETFONT, reinterpret_cast<WPARAM>(g_ui_font), TRUE);
        }
        subclass_edit(g_options_size);
        subclass_edit(g_options_hardness);
        sync_options_controls();
        return 0;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps{};
        HDC dc = BeginPaint(hwnd, &ps);
        draw_options_panel(hwnd, dc);
        EndPaint(hwnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1;
    case WM_DRAWITEM:
        draw_owner_button(reinterpret_cast<DRAWITEMSTRUCT*>(lparam));
        return TRUE;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT: {
        HDC dc = reinterpret_cast<HDC>(wparam);
        SetTextColor(dc, RGB(235, 241, 248));
        SetBkColor(dc, RGB(20, 25, 33));
        return reinterpret_cast<LRESULT>(message == WM_CTLCOLOREDIT ? g_edit_brush : g_panel_brush);
    }
    case WM_COMMAND:
        if (LOWORD(wparam) == ID_OPTIONS && HIWORD(wparam) == EN_CHANGE) {
            g_app.brush_size = clamp_int(get_control_int(reinterpret_cast<HWND>(lparam), g_app.brush_size), 1, 128);
            set_control_int_if_changed(g_brush_size, g_app.brush_size);
            invalidate_tool_ui();
            InvalidateRect(hwnd, nullptr, FALSE);
            set_status(L"Tamanho do pincel atualizado.");
        } else if (LOWORD(wparam) == ID_HARDNESS && HIWORD(wparam) == EN_CHANGE) {
            g_app.brush_hardness = clamp_int(get_control_int(reinterpret_cast<HWND>(lparam), g_app.brush_hardness), 1, 100);
            invalidate_tool_ui();
            InvalidateRect(hwnd, nullptr, FALSE);
            set_status(L"Dureza do pincel atualizada.");
        } else if (LOWORD(wparam) == ID_SHAPE_ROUND) {
            g_app.brush_shape = BrushShape::Round;
            sync_options_controls();
            InvalidateRect(hwnd, nullptr, FALSE);
            set_status(L"Pincel redondo selecionado.");
        } else if (LOWORD(wparam) == ID_SHAPE_SQUARE) {
            g_app.brush_shape = BrushShape::Square;
            sync_options_controls();
            InvalidateRect(hwnd, nullptr, FALSE);
            set_status(L"Pincel quadrado selecionado.");
        } else if (LOWORD(wparam) == ID_COLOR_PICK) {
            choose_brush_color(hwnd);
            InvalidateRect(hwnd, nullptr, FALSE);
        } else if (LOWORD(wparam) == ID_OPTIONS_CLOSE) {
            DestroyWindow(hwnd);
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        g_options_window = nullptr;
        g_options_size = nullptr;
        g_options_hardness = nullptr;
        g_shape_round = nullptr;
        g_shape_square = nullptr;
        return 0;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

void show_options_window(POINT screen_pos = POINT{-1, -1}) {
    RECT client{};
    GetClientRect(g_main, &client);
    Rect canvas = canvas_rect(g_main);
    constexpr int panel_w = 410;
    constexpr int panel_h = 210;
    int x = screen_pos.x >= 0 ? screen_pos.x : canvas.X + 340;
    int y = screen_pos.y >= 0 ? screen_pos.y : canvas.Y + 260;
    x = clamp_int(x, canvas.X + 12, std::max(canvas.X + 12, static_cast<int>(client.right) - panel_w - UI_RIGHT_PANEL_W - 20));
    y = clamp_int(y, canvas.Y + 12, std::max(canvas.Y + 12, static_cast<int>(client.bottom) - UI_STATUS_H - panel_h - 12));
    g_app.tool_panel_open = true;
    g_app.tool_panel_rect = Rect(x, y, panel_w, panel_h);
    layout_tool_panel_controls(g_main);
    set_status(L"Painel da ferramenta aberto dentro do editor.");
}

void choose_brush_color(HWND owner) {
    static COLORREF custom_colors[16] = {};
    CHOOSECOLORW chooser = {};
    chooser.lStructSize = sizeof(chooser);
    chooser.hwndOwner = owner;
    chooser.rgbResult = g_app.brush_color;
    chooser.lpCustColors = custom_colors;
    chooser.Flags = CC_FULLOPEN | CC_RGBINIT;
    if (ChooseColorW(&chooser)) {
        g_app.brush_color = chooser.rgbResult;
        invalidate_tool_ui();
        set_status(L"Cor do lapis atualizada.");
    }
}

void subclass_edit(HWND hwnd) {
    if (!g_edit_proc) {
        g_edit_proc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(hwnd, GWLP_WNDPROC));
    }
    SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(edit_proc));
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE: {
        g_main = hwnd;
        g_ui_font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");
        g_panel_brush = CreateSolidBrush(RGB(20, 25, 33));
        g_edit_brush = CreateSolidBrush(RGB(16, 20, 27));
        g_open = make_button(hwnd, L"Abrir PNG", BS_PUSHBUTTON, ID_OPEN);
        g_save_png = make_button(hwnd, L"Salvar PNG", BS_PUSHBUTTON, ID_SAVE_PNG);
        g_reset = make_button(hwnd, L"Template padrao", BS_PUSHBUTTON, ID_RESET);
        g_select_tool = make_button(hwnd, L"Selecionar", BS_AUTORADIOBUTTON, ID_TOOL_SELECT);
        g_pencil_tool = make_button(hwnd, L"Lapis", BS_AUTORADIOBUTTON, ID_TOOL_PENCIL);
        g_eraser_tool = make_button(hwnd, L"Borracha", BS_AUTORADIOBUTTON, ID_TOOL_ERASER);
        g_color_pick = make_button(hwnd, L"Cor", BS_PUSHBUTTON, ID_COLOR_PICK);
        g_zoom_out = make_button(hwnd, L"-", BS_PUSHBUTTON, ID_ZOOM_OUT);
        g_zoom_label = make_child(hwnd, L"STATIC", L"Fit", SS_CENTER, 0);
        g_zoom_in = make_button(hwnd, L"+", BS_PUSHBUTTON, ID_ZOOM_IN);
        g_zoom_fit = make_button(hwnd, L"Fit", BS_PUSHBUTTON, ID_ZOOM_FIT);
        g_options = make_button(hwnd, L"Opcoes", BS_PUSHBUTTON, ID_OPTIONS);
        g_dev_check = make_button(hwnd, L"Modo-dev", BS_AUTOCHECKBOX, ID_DEV);
        SendMessageW(g_dev_check, BM_SETCHECK, BST_UNCHECKED, 0);
        g_show_all = make_button(hwnd, L"Mostrar todas partes", BS_AUTOCHECKBOX, ID_SHOW_ALL);
        g_templates = make_child(hwnd, L"LISTBOX", L"", LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_BORDER | WS_VSCROLL, ID_TEMPLATES);
        g_parts = make_child(hwnd, L"LISTBOX", L"", LBS_NOTIFY | LBS_OWNERDRAWFIXED | LBS_HASSTRINGS | WS_BORDER | WS_VSCROLL, ID_PARTS);
        SendMessageW(g_templates, LB_SETITEMHEIGHT, 0, 25);
        SendMessageW(g_parts, LB_SETITEMHEIGHT, 0, 34);
        g_status = make_child(hwnd, L"STATIC", L"Pronto.", SS_LEFT, 0);

        make_child(hwnd, L"STATIC", L"ID", SS_LEFT, 2001);
        make_child(hwnd, L"STATIC", L"Label", SS_LEFT, 2002);
        make_child(hwnd, L"STATIC", L"X", SS_LEFT, 2003);
        make_child(hwnd, L"STATIC", L"Y", SS_LEFT, 2004);
        make_child(hwnd, L"STATIC", L"W", SS_LEFT, 2005);
        make_child(hwnd, L"STATIC", L"H", SS_LEFT, 2006);
        make_child(hwnd, L"STATIC", L"Pincel", SS_LEFT, 2007);
        g_id = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 0);
        g_label = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 0);
        g_x = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_y = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_w = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_h = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_brush_size = make_child(hwnd, L"EDIT", L"8", WS_BORDER | ES_NUMBER, ID_BRUSH_SIZE);
        g_tool_preview = make_child(hwnd, L"STATIC", L"", SS_OWNERDRAW, ID_TOOL_PREVIEW);
        g_apply = make_button(hwnd, L"Aplicar parte", BS_PUSHBUTTON, ID_APPLY);
        g_new_part = make_button(hwnd, L"Nova parte", BS_PUSHBUTTON, ID_NEW);
        g_delete_part = make_button(hwnd, L"Remover parte", BS_PUSHBUTTON, ID_DELETE);
        g_part_up = make_button(hwnd, L"Subir", BS_PUSHBUTTON, ID_PART_UP);
        g_part_down = make_button(hwnd, L"Descer", BS_PUSHBUTTON, ID_PART_DOWN);
        g_save = make_button(hwnd, L"Salvar JSON", BS_PUSHBUTTON, ID_SAVE);
        g_options_size = make_child(hwnd, L"EDIT", L"8", WS_BORDER | ES_NUMBER, ID_OPTIONS_SIZE);
        g_options_hardness = make_child(hwnd, L"EDIT", L"100", WS_BORDER | ES_NUMBER, ID_HARDNESS);
        g_shape_round = make_button(hwnd, L"Redondo", BS_AUTORADIOBUTTON, ID_SHAPE_ROUND);
        g_shape_square = make_button(hwnd, L"Quadrado", BS_AUTORADIOBUTTON, ID_SHAPE_SQUARE);
        g_options_color = make_button(hwnd, L"Cor", BS_PUSHBUTTON, ID_OPTIONS_COLOR);
        g_options_close = make_button(hwnd, L"X", BS_PUSHBUTTON, ID_OPTIONS_CLOSE);
        HWND edits[] = {g_id, g_label, g_x, g_y, g_w, g_h, g_brush_size, g_options_size, g_options_hardness};
        for (HWND edit : edits) {
            subclass_edit(edit);
        }
        g_parts_proc = reinterpret_cast<WNDPROC>(GetWindowLongPtrW(g_parts, GWLP_WNDPROC));
        SetWindowLongPtrW(g_parts, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(parts_list_proc));
        set_tool(Tool::Select);
        update_zoom_label();

        rebuild_templates();
        load_default_image();
        layout(hwnd);
        return 0;
    }
    case WM_SIZE:
        layout(hwnd);
        return 0;
    case WM_ERASEBKGND:
        return 1;
    case WM_DRAWITEM:
        if (reinterpret_cast<DRAWITEMSTRUCT*>(lparam)->CtlID == ID_TOOL_PREVIEW) {
            draw_tool_preview(reinterpret_cast<DRAWITEMSTRUCT*>(lparam));
        } else {
            draw_owner_button(reinterpret_cast<DRAWITEMSTRUCT*>(lparam));
        }
        return TRUE;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        HDC dc = reinterpret_cast<HDC>(wparam);
        SetTextColor(dc, RGB(235, 241, 248));
        SetBkColor(dc, RGB(20, 25, 33));
        return reinterpret_cast<LRESULT>(message == WM_CTLCOLOREDIT ? g_edit_brush : g_panel_brush);
    }
    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        if (g_app.tool_panel_open) {
            int mx = GET_X_LPARAM(lparam);
            int my = GET_Y_LPARAM(lparam);
            Rect panel = g_app.tool_panel_rect;
            bool inside_panel = mx >= panel.X && my >= panel.Y && mx <= panel.X + panel.Width && my <= panel.Y + panel.Height;
            if (!inside_panel) {
                close_tool_panel();
            } else {
                return 0;
            }
        }
        if (g_app.tool == Tool::Select) {
            select_part_from_point(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        } else {
            push_undo_snapshot();
            g_app.drawing = true;
            g_app.last_mouse = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
            SetCapture(hwnd);
            paint_stroke(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam), g_app.tool);
        }
        return 0;
    case WM_LBUTTONUP:
        if (g_app.drawing) {
            g_app.drawing = false;
            ReleaseCapture();
            set_status(L"Edicao aplicada na imagem. Use Salvar PNG para exportar.");
        }
        return 0;
    case WM_RBUTTONDOWN:
        if (GetKeyState(VK_MENU) & 0x8000) {
            g_app.panning = true;
            g_app.last_mouse = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
            SetCapture(hwnd);
        } else {
            POINT screen_pos{GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
            show_options_window(screen_pos);
        }
        return 0;
    case WM_RBUTTONUP:
        if (g_app.panning) {
            g_app.panning = false;
            ReleaseCapture();
        }
        return 0;
    case WM_MBUTTONDOWN:
        g_app.panning = true;
        g_app.last_mouse = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        SetCapture(hwnd);
        return 0;
    case WM_MBUTTONUP:
        if (g_app.panning) {
            g_app.panning = false;
            ReleaseCapture();
        }
        return 0;
    case WM_MOUSEMOVE:
    {
        POINT old_hover = g_app.hover_mouse;
        bool old_hover_canvas = g_app.hover_canvas;
        g_app.hover_mouse = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        Rect canvas = canvas_rect(hwnd);
        g_app.hover_canvas = g_app.hover_mouse.x >= canvas.X
            && g_app.hover_mouse.y >= canvas.Y
            && g_app.hover_mouse.x <= canvas.X + canvas.Width
            && g_app.hover_mouse.y <= canvas.Y + canvas.Height;
        if (!g_app.tracking_mouse) {
            TRACKMOUSEEVENT event{};
            event.cbSize = sizeof(event);
            event.dwFlags = TME_LEAVE;
            event.hwndTrack = hwnd;
            TrackMouseEvent(&event);
            g_app.tracking_mouse = true;
        }
        if (g_app.drawing) {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            paint_stroke(g_app.last_mouse.x, g_app.last_mouse.y, x, y, g_app.tool);
            g_app.last_mouse = {x, y};
        } else if (g_app.panning) {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            g_app.pan_x += x - g_app.last_mouse.x;
            g_app.pan_y += y - g_app.last_mouse.y;
            g_app.last_mouse = {x, y};
            g_app.fit_to_view = false;
            update_zoom_label();
            InvalidateRect(hwnd, nullptr, FALSE);
        } else {
            int hover_size = std::max(48, g_app.brush_size + 24);
            if (old_hover_canvas) {
                RECT old_rect{old_hover.x - hover_size, old_hover.y - hover_size, old_hover.x + hover_size, old_hover.y + hover_size};
                InvalidateRect(hwnd, &old_rect, FALSE);
            }
            if (g_app.hover_canvas) {
                RECT new_rect{g_app.hover_mouse.x - hover_size, g_app.hover_mouse.y - hover_size, g_app.hover_mouse.x + hover_size, g_app.hover_mouse.y + hover_size};
                InvalidateRect(hwnd, &new_rect, FALSE);
            }
            RECT hud_rect{canvas.X + 8, canvas.Y + canvas.Height - 42, canvas.X + 390, canvas.Y + canvas.Height - 4};
            InvalidateRect(hwnd, &hud_rect, FALSE);
        }
        return 0;
    }
    case WM_MOUSELEAVE:
        g_app.hover_canvas = false;
        g_app.tracking_mouse = false;
        InvalidateRect(hwnd, nullptr, FALSE);
        return 0;
    case WM_MOUSEWHEEL:
        if (GET_KEYSTATE_WPARAM(wparam) & MK_CONTROL) {
            int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            POINT zoom_point{GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
            ScreenToClient(hwnd, &zoom_point);
            set_zoom_at((g_app.fit_to_view ? 1.0 : g_app.zoom) * (delta > 0 ? 1.25 : 0.8), zoom_point.x, zoom_point.y);
            return 0;
        }
        return 0;
    case WM_KEYDOWN:
        if (GetKeyState(VK_CONTROL) & 0x8000) {
            if (wparam == 'Z') {
                undo_image();
                return 0;
            }
            if (wparam == 'Y') {
                redo_image();
                return 0;
            }
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case ID_BRUSH_SIZE:
            if (HIWORD(wparam) == EN_CHANGE) {
                g_app.brush_size = clamp_int(get_control_int(g_brush_size, g_app.brush_size), 1, 128);
                if (g_options_size) {
                    set_control_int_if_changed(g_options_size, g_app.brush_size);
                }
                invalidate_tool_ui();
            }
            return 0;
        case ID_OPTIONS_SIZE:
            if (HIWORD(wparam) == EN_CHANGE) {
                g_app.brush_size = clamp_int(get_control_int(g_options_size, g_app.brush_size), 1, 128);
                set_control_int_if_changed(g_brush_size, g_app.brush_size);
                invalidate_tool_ui();
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case ID_HARDNESS:
            if (HIWORD(wparam) == EN_CHANGE) {
                g_app.brush_hardness = clamp_int(get_control_int(g_options_hardness, g_app.brush_hardness), 1, 100);
                invalidate_tool_ui();
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case ID_SHAPE_ROUND:
            g_app.brush_shape = BrushShape::Round;
            sync_options_controls();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case ID_SHAPE_SQUARE:
            g_app.brush_shape = BrushShape::Square;
            sync_options_controls();
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case ID_OPTIONS_COLOR:
            choose_brush_color(hwnd);
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case ID_OPTIONS_CLOSE:
            close_tool_panel();
            return 0;
        case ID_TEMPLATES:
            if (HIWORD(wparam) == LBN_SELCHANGE) {
                close_tool_panel();
                int next_template = static_cast<int>(SendMessageW(g_templates, LB_GETCURSEL, 0, 0));
                if (next_template == g_app.selected_template) {
                    return 0;
                }
                g_app.selected_template = next_template;
                g_app.selected_part = 0;
                rebuild_parts();
                load_default_image();
            }
            return 0;
        case ID_PARTS:
            if (HIWORD(wparam) == LBN_SELCHANGE) {
                close_tool_panel();
                g_app.selected_part = static_cast<int>(SendMessageW(g_parts, LB_GETCURSEL, 0, 0));
                populate_dev_fields();
                InvalidateRect(hwnd, nullptr, FALSE);
            }
            return 0;
        case ID_OPEN:
            choose_image(hwnd);
            return 0;
        case ID_SAVE_PNG:
            save_image_as(hwnd);
            return 0;
        case ID_RESET:
            load_default_image(true);
            return 0;
        case ID_TOOL_SELECT:
            close_tool_panel();
            set_tool(Tool::Select);
            return 0;
        case ID_TOOL_PENCIL:
            close_tool_panel();
            set_tool(Tool::Pencil);
            return 0;
        case ID_TOOL_ERASER:
            close_tool_panel();
            set_tool(Tool::Eraser);
            return 0;
        case ID_COLOR_PICK:
            choose_brush_color(hwnd);
            return 0;
        case ID_ZOOM_OUT:
            set_zoom((g_app.fit_to_view ? 1.0 : g_app.zoom) / 2.0);
            return 0;
        case ID_ZOOM_IN:
            set_zoom((g_app.fit_to_view ? 1.0 : g_app.zoom) * 2.0);
            return 0;
        case ID_ZOOM_FIT:
            fit_zoom();
            return 0;
        case ID_OPTIONS:
            return 0;
        case ID_DEV:
            g_app.dev_mode = !g_app.dev_mode;
            SendMessageW(g_dev_check, BM_SETCHECK, g_app.dev_mode ? BST_CHECKED : BST_UNCHECKED, 0);
            layout(hwnd);
            invalidate_tool_ui();
            return 0;
        case ID_SHOW_ALL:
            g_app.show_all_parts = !g_app.show_all_parts;
            SendMessageW(g_show_all, BM_SETCHECK, g_app.show_all_parts ? BST_CHECKED : BST_UNCHECKED, 0);
            set_status(g_app.show_all_parts ? L"Mostrando todas as parts." : L"Mostrando somente a part focada.");
            RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            return 0;
        case ID_APPLY:
            apply_dev_fields();
            invalidate_tool_ui();
            return 0;
        case ID_NEW:
            add_part();
            return 0;
        case ID_DELETE:
            delete_part();
            return 0;
        case ID_PART_UP:
            move_selected_part(-1);
            return 0;
        case ID_PART_DOWN:
            move_selected_part(1);
            return 0;
        case ID_SAVE:
            save_templates();
            return 0;
        }
        break;
    case WM_PAINT:
        paint(hwnd);
        return 0;
    case WM_DESTROY:
        if (g_ui_font) {
            DeleteObject(g_ui_font);
            g_ui_font = nullptr;
        }
        if (g_panel_brush) {
            DeleteObject(g_panel_brush);
            g_panel_brush = nullptr;
        }
        if (g_edit_brush) {
            DeleteObject(g_edit_brush);
            g_edit_brush = nullptr;
        }
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, message, wparam, lparam);
}

int APIENTRY wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR, int show) {
    Gdiplus::GdiplusStartupInput gdiplus_startup_input;
    Gdiplus::GdiplusStartup(&g_app.gdiplus_token, &gdiplus_startup_input, nullptr);

    try {
        g_app.root = app_root();
        g_app.template_path = g_app.root / "data" / "templates" / "teeworlds_textures.json";
        g_app.templates = parse_templates(read_file(g_app.template_path));
    } catch (const std::exception& error) {
        MessageBoxA(nullptr, error.what(), "Teeworlds Texture Editor", MB_ICONERROR);
        return 1;
    }

    WNDCLASSW wc = {};
    wc.lpfnWndProc = window_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(instance, MAKEINTRESOURCE(101));
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"TeeworldsTextureEditorWindow";
    RegisterClassW(&wc);

    g_main = CreateWindowExW(
        0,
        wc.lpszClassName,
        L"Teeworlds Texture Editor 0.1 - Lex copyright 2026",
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1700,
        900,
        nullptr,
        nullptr,
        instance,
        nullptr);

    if (!g_main) {
        return 1;
    }

    ShowWindow(g_main, show);
    UpdateWindow(g_main);

    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_KEYDOWN && (GetKeyState(VK_CONTROL) & 0x8000)) {
            if (msg.wParam == 'Z') {
                undo_image();
                continue;
            }
            if (msg.wParam == 'Y') {
                redo_image();
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_app.gdiplus_token != 0) {
        Gdiplus::GdiplusShutdown(g_app.gdiplus_token);
    }
    return static_cast<int>(msg.wParam);
}

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
    bool dev_mode = true;
    bool show_all_parts = false;
    bool fit_to_view = true;
    bool drawing = false;
    bool panning = false;
    Tool tool = Tool::Select;
    int brush_size = 8;
    double zoom = 1.0;
    int pan_x = 0;
    int pan_y = 0;
    POINT last_mouse{};
    std::unique_ptr<Bitmap> image;
    Rect image_rect;
    Rect preview_rect;
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
HWND g_save = nullptr;
HWND g_open = nullptr;
HWND g_save_png = nullptr;
HWND g_reset = nullptr;
HWND g_show_all = nullptr;
HWND g_select_tool = nullptr;
HWND g_pencil_tool = nullptr;
HWND g_eraser_tool = nullptr;
HWND g_zoom_out = nullptr;
HWND g_zoom_label = nullptr;
HWND g_zoom_in = nullptr;
HWND g_zoom_fit = nullptr;
HWND g_brush_size = nullptr;
HFONT g_ui_font = nullptr;
WNDPROC g_edit_proc = nullptr;

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
    }
    return CallWindowProcW(g_edit_proc, hwnd, message, wparam, lparam);
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
    g_app.selected_part = texture->parts.empty() ? -1 : std::clamp(g_app.selected_part, 0, static_cast<int>(texture->parts.size()) - 1);
    if (g_app.selected_part >= 0) {
        SendMessageW(g_parts, LB_SETCURSEL, g_app.selected_part, 0);
    }
    populate_dev_fields();
    InvalidateRect(g_main, nullptr, FALSE);
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
    int left = 320;
    int top = 56;
    int status_h = 28;
    int dev = g_app.dev_mode ? 285 : 0;
    int gap = 10;

    MoveWindow(g_open, 12, 12, 110, 30, TRUE);
    MoveWindow(g_save_png, 130, 12, 110, 30, TRUE);
    MoveWindow(g_reset, 248, 12, 125, 30, TRUE);
    MoveWindow(g_select_tool, 390, 12, 90, 30, TRUE);
    MoveWindow(g_pencil_tool, 488, 12, 80, 30, TRUE);
    MoveWindow(g_eraser_tool, 576, 12, 90, 30, TRUE);
    MoveWindow(g_zoom_out, 690, 12, 34, 30, TRUE);
    MoveWindow(g_zoom_label, 730, 16, 58, 24, TRUE);
    MoveWindow(g_zoom_in, 794, 12, 34, 30, TRUE);
    MoveWindow(g_zoom_fit, 834, 12, 48, 30, TRUE);
    MoveWindow(g_show_all, 900, 16, 160, 24, TRUE);
    MoveWindow(g_dev_check, width - 150, 16, 135, 24, TRUE);
    MoveWindow(g_templates, 12, top, left - 24, 170, TRUE);
    MoveWindow(g_parts, 12, top + 185, left - 24, height - top - 198 - status_h, TRUE);
    MoveWindow(g_status, 0, height - status_h, width, status_h, TRUE);

    int right_x = width - dev + 12;
    int y = top;
    auto place = [&](HWND label, HWND edit) {
        MoveWindow(label, right_x, y, 70, 22, TRUE);
        MoveWindow(edit, right_x + 78, y, dev - 100, 24, TRUE);
        y += 34;
    };

    HWND children[] = {g_id, g_label, g_x, g_y, g_w, g_h, g_brush_size, g_apply, g_new_part, g_delete_part, g_save};
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
        ShowWindow(brush_label, SW_SHOW);
        place(labels[0], g_id);
        place(labels[1], g_label);
        place(labels[2], g_x);
        place(labels[3], g_y);
        place(labels[4], g_w);
        place(labels[5], g_h);
        place(brush_label, g_brush_size);
        MoveWindow(g_apply, right_x, y + gap, dev - 24, 30, TRUE);
        MoveWindow(g_new_part, right_x, y + 46, dev - 24, 30, TRUE);
        MoveWindow(g_delete_part, right_x, y + 82, dev - 24, 30, TRUE);
        MoveWindow(g_save, right_x, y + 128, dev - 24, 34, TRUE);
    } else {
        for (int id = 2001; id <= 2006; ++id) {
            ShowWindow(GetDlgItem(hwnd, id), SW_HIDE);
        }
        ShowWindow(GetDlgItem(hwnd, 2007), SW_HIDE);
    }
    InvalidateRect(hwnd, nullptr, TRUE);
}

Rect canvas_rect(HWND hwnd) {
    RECT client{};
    GetClientRect(hwnd, &client);
    int left = 330;
    int top = 56;
    int right_panel = g_app.dev_mode ? 295 : 10;
    return Rect(left, top, std::max(50, static_cast<int>(client.right) - left - right_panel), std::max(50, static_cast<int>(client.bottom) - top - 38));
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
    return Rect(
        g_app.image_rect.X + static_cast<int>(part.x * sx),
        g_app.image_rect.Y + static_cast<int>(part.y * sy),
        std::max(1, static_cast<int>(part.w * sx)),
        std::max(1, static_cast<int>(part.h * sy)));
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
    InvalidateRect(g_select_tool, nullptr, TRUE);
    InvalidateRect(g_pencil_tool, nullptr, TRUE);
    InvalidateRect(g_eraser_tool, nullptr, TRUE);
    const wchar_t* name = tool == Tool::Select ? L"Selecionar" : (tool == Tool::Pencil ? L"Lapis" : L"Borracha");
    set_status(std::wstring(L"Ferramenta: ") + name);
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

void fit_zoom() {
    g_app.fit_to_view = true;
    g_app.pan_x = 0;
    g_app.pan_y = 0;
    update_zoom_label();
    InvalidateRect(g_main, nullptr, FALSE);
}

void paint_at(int screen_x, int screen_y) {
    POINT p = screen_to_image_point(screen_x, screen_y);
    if (!image_point_inside(p)) {
        return;
    }
    g_app.brush_size = clamp_int(get_control_int(g_brush_size, g_app.brush_size), 1, 128);
    Graphics image_g(g_app.image.get());
    image_g.SetSmoothingMode(Gdiplus::SmoothingModeNone);
    image_g.SetCompositingMode(g_app.tool == Tool::Eraser ? CompositingModeSourceCopy : Gdiplus::CompositingModeSourceOver);
    Color color = g_app.tool == Tool::Eraser ? Color(0, 0, 0, 0) : Color(255, 255, 255, 255);
    SolidBrush brush(color);
    int radius = std::max(1, g_app.brush_size);
    image_g.FillRectangle(&brush, p.x - radius / 2, p.y - radius / 2, radius, radius);

    Rect dirty = image_rect_to_screen_rect(p.x - radius, p.y - radius, radius * 2, radius * 2);
    RECT dirty_win = inflated_win_rect(dirty, 4);
    InvalidateRect(g_main, &dirty_win, FALSE);
    if (g_app.preview_rect.Width > 0 && g_app.preview_rect.Height > 0) {
        RECT preview = inflated_win_rect(g_app.preview_rect, 4);
        InvalidateRect(g_main, &preview, FALSE);
    }
}

void render_scene(HWND hwnd, Graphics& g, const RECT& client, const RECT& repaint) {
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    g.Clear(Color(255, 18, 22, 29));

    SolidBrush top(Color(255, 27, 33, 43));
    g.FillRectangle(&top, 0, 0, client.right, 56);

    SolidBrush panel(Color(255, 24, 29, 38));
    g.FillRectangle(&panel, 0, 56, 320, client.bottom);
    if (g_app.dev_mode) {
        g.FillRectangle(&panel, client.right - 295, 56, 295, client.bottom);
    }

    Rect canvas = canvas_rect(hwnd);
    SolidBrush canvas_brush(Color(255, 12, 14, 18));
    g.FillRectangle(&canvas_brush, canvas);

    SolidBrush checker_a(Color(255, 26, 30, 38));
    SolidBrush checker_b(Color(255, 33, 38, 48));
    RECT canvas_win = to_win_rect(canvas);
    RECT checker_clip{};
    IntersectRect(&checker_clip, &repaint, &canvas_win);
    constexpr int checker = 16;
    int start_y = canvas.Y + ((checker_clip.top - canvas.Y) / checker) * checker;
    int start_x = canvas.X + ((checker_clip.left - canvas.X) / checker) * checker;
    for (int y = start_y; y < checker_clip.bottom; y += checker) {
        for (int x = start_x; x < checker_clip.right; x += checker) {
            bool alt = ((x / checker) + (y / checker)) % 2 == 0;
            g.FillRectangle(alt ? &checker_a : &checker_b, x, y, checker, checker);
        }
    }

    Pen frame(Color(180, 92, 205, 255), 2.0f);
    g.DrawRectangle(&frame, canvas);

    if (g_app.image) {
        g_app.image_rect = fit_image_rect(canvas, g_app.image.get());
        g.DrawImage(g_app.image.get(), g_app.image_rect);

        Part* part = current_part();
        if (part) {
            Rect highlight = part_to_screen(*part);
            SolidBrush dim(Color(145, 0, 0, 0));
            Gdiplus::Region dim_region(g_app.image_rect);
            dim_region.Exclude(highlight);
            g.FillRegion(&dim, &dim_region);

            Pen cyan(Color(255, 85, 220, 255), 3.0f);
            if (g_app.show_all_parts) {
                TextureTemplate* texture = current_template();
                if (texture) {
                    Pen soft(Color(150, 85, 220, 255), 1.0f);
                    for (const Part& each : texture->parts) {
                        g.DrawRectangle(&soft, part_to_screen(each));
                    }
                }
            }
            g.DrawRectangle(&cyan, highlight);

            int max_preview_w = std::max(96, canvas.Width / 3);
            int max_preview_h = std::max(96, canvas.Height / 3);
            int zoom = std::max(1, std::min({8, max_preview_w / std::max(1, part->w), max_preview_h / std::max(1, part->h)}));
            int preview_w = std::max(1, part->w * zoom);
            int preview_h = std::max(1, part->h * zoom);
            Rect preview(canvas.X + canvas.Width - preview_w - 18, canvas.Y + canvas.Height - preview_h - 18, preview_w, preview_h);
            g_app.preview_rect = preview;
            SolidBrush preview_bg(Color(230, 20, 25, 33));
            g.FillRectangle(&preview_bg, preview);
            g.DrawRectangle(&cyan, preview);
            Rect src(part->x, part->y, part->w, part->h);
            g.DrawImage(g_app.image.get(), preview, src.X, src.Y, src.Width, src.Height, Gdiplus::UnitPixel);
        } else {
            g_app.preview_rect = Rect();
        }
    } else {
        g_app.preview_rect = Rect();
    }

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
        if (ix >= part.x && ix <= part.x + part.w && iy >= part.y && iy <= part.y + part.h) {
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

bool is_checked_button(int id) {
    if (id == ID_DEV || id == ID_SHOW_ALL || id == ID_TOOL_SELECT || id == ID_TOOL_PENCIL || id == ID_TOOL_ERASER) {
        return SendMessageW(GetDlgItem(g_main, id), BM_GETCHECK, 0, 0) == BST_CHECKED;
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
    RECT r = item->rcItem;
    bool pressed = (item->itemState & ODS_SELECTED) != 0;
    bool checked = is_checked_button(static_cast<int>(item->CtlID));
    bool disabled = (item->itemState & ODS_DISABLED) != 0;

    COLORREF bg = checked ? RGB(55, 124, 226) : (pressed ? RGB(45, 52, 66) : RGB(31, 37, 48));
    COLORREF border = checked ? RGB(91, 220, 255) : RGB(75, 86, 104);
    COLORREF text = disabled ? RGB(120, 128, 140) : RGB(235, 241, 248);

    HBRUSH bg_brush = CreateSolidBrush(bg);
    HPEN border_pen = CreatePen(PS_SOLID, 1, border);
    HGDIOBJ old_brush = SelectObject(item->hDC, bg_brush);
    HGDIOBJ old_pen = SelectObject(item->hDC, border_pen);
    RoundRect(item->hDC, r.left, r.top, r.right, r.bottom, 7, 7);
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
        r.left += 28;
    }

    wchar_t label[128] = {};
    GetWindowTextW(item->hwndItem, label, 128);
    DrawTextW(item->hDC, label, -1, &r, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
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
        g_open = make_button(hwnd, L"Abrir PNG", BS_PUSHBUTTON, ID_OPEN);
        g_save_png = make_button(hwnd, L"Salvar PNG", BS_PUSHBUTTON, ID_SAVE_PNG);
        g_reset = make_button(hwnd, L"Template padrao", BS_PUSHBUTTON, ID_RESET);
        g_select_tool = make_button(hwnd, L"Selecionar", BS_AUTORADIOBUTTON, ID_TOOL_SELECT);
        g_pencil_tool = make_button(hwnd, L"Lapis", BS_AUTORADIOBUTTON, ID_TOOL_PENCIL);
        g_eraser_tool = make_button(hwnd, L"Borracha", BS_AUTORADIOBUTTON, ID_TOOL_ERASER);
        g_zoom_out = make_button(hwnd, L"-", BS_PUSHBUTTON, ID_ZOOM_OUT);
        g_zoom_label = make_child(hwnd, L"STATIC", L"Fit", SS_CENTER, 0);
        g_zoom_in = make_button(hwnd, L"+", BS_PUSHBUTTON, ID_ZOOM_IN);
        g_zoom_fit = make_button(hwnd, L"Fit", BS_PUSHBUTTON, ID_ZOOM_FIT);
        g_dev_check = make_button(hwnd, L"Modo-dev", BS_AUTOCHECKBOX, ID_DEV);
        SendMessageW(g_dev_check, BM_SETCHECK, BST_CHECKED, 0);
        g_show_all = make_button(hwnd, L"Mostrar todas partes", BS_AUTOCHECKBOX, ID_SHOW_ALL);
        g_templates = make_child(hwnd, L"LISTBOX", L"", LBS_NOTIFY | WS_BORDER | WS_VSCROLL, ID_TEMPLATES);
        g_parts = make_child(hwnd, L"LISTBOX", L"", LBS_NOTIFY | WS_BORDER | WS_VSCROLL, ID_PARTS);
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
        g_brush_size = make_child(hwnd, L"EDIT", L"8", WS_BORDER | ES_NUMBER, 0);
        g_apply = make_button(hwnd, L"Aplicar parte", BS_PUSHBUTTON, ID_APPLY);
        g_new_part = make_button(hwnd, L"Nova parte", BS_PUSHBUTTON, ID_NEW);
        g_delete_part = make_button(hwnd, L"Remover parte", BS_PUSHBUTTON, ID_DELETE);
        g_save = make_button(hwnd, L"Salvar JSON", BS_PUSHBUTTON, ID_SAVE);
        HWND edits[] = {g_id, g_label, g_x, g_y, g_w, g_h, g_brush_size};
        for (HWND edit : edits) {
            subclass_edit(edit);
        }
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
        draw_owner_button(reinterpret_cast<DRAWITEMSTRUCT*>(lparam));
        return TRUE;
    case WM_LBUTTONDOWN:
        SetFocus(hwnd);
        if (g_app.tool == Tool::Select) {
            select_part_from_point(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        } else {
            g_app.drawing = true;
            g_app.last_mouse = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
            SetCapture(hwnd);
            paint_at(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
            UpdateWindow(hwnd);
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
        g_app.panning = true;
        g_app.last_mouse = {GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam)};
        SetCapture(hwnd);
        return 0;
    case WM_RBUTTONUP:
        if (g_app.panning) {
            g_app.panning = false;
            ReleaseCapture();
        }
        return 0;
    case WM_MOUSEMOVE:
        if (g_app.drawing) {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            int dx = x - g_app.last_mouse.x;
            int dy = y - g_app.last_mouse.y;
            int steps = std::max(1, std::max(std::abs(dx), std::abs(dy)) / std::max(1, g_app.brush_size / 2));
            for (int i = 1; i <= steps; ++i) {
                int px = g_app.last_mouse.x + dx * i / steps;
                int py = g_app.last_mouse.y + dy * i / steps;
                paint_at(px, py);
            }
            g_app.last_mouse = {x, y};
            UpdateWindow(hwnd);
        } else if (g_app.panning) {
            int x = GET_X_LPARAM(lparam);
            int y = GET_Y_LPARAM(lparam);
            g_app.pan_x += x - g_app.last_mouse.x;
            g_app.pan_y += y - g_app.last_mouse.y;
            g_app.last_mouse = {x, y};
            g_app.fit_to_view = false;
            update_zoom_label();
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    case WM_MOUSEWHEEL:
        if (GET_KEYSTATE_WPARAM(wparam) & MK_CONTROL) {
            int delta = GET_WHEEL_DELTA_WPARAM(wparam);
            set_zoom((g_app.fit_to_view ? 1.0 : g_app.zoom) * (delta > 0 ? 1.25 : 0.8));
            return 0;
        }
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case ID_TEMPLATES:
            if (HIWORD(wparam) == LBN_SELCHANGE) {
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
            set_tool(Tool::Select);
            return 0;
        case ID_TOOL_PENCIL:
            set_tool(Tool::Pencil);
            return 0;
        case ID_TOOL_ERASER:
            set_tool(Tool::Eraser);
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
        case ID_DEV:
            g_app.dev_mode = SendMessageW(g_dev_check, BM_GETCHECK, 0, 0) == BST_CHECKED;
            layout(hwnd);
            return 0;
        case ID_SHOW_ALL:
            g_app.show_all_parts = SendMessageW(g_show_all, BM_GETCHECK, 0, 0) == BST_CHECKED;
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        case ID_APPLY:
            apply_dev_fields();
            return 0;
        case ID_NEW:
            add_part();
            return 0;
        case ID_DELETE:
            delete_part();
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
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
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
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    if (g_app.gdiplus_token != 0) {
        Gdiplus::GdiplusShutdown(g_app.gdiplus_token);
    }
    return static_cast<int>(msg.wParam);
}

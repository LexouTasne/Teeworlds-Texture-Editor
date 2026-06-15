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
using Gdiplus::Graphics;
using Gdiplus::Image;
using Gdiplus::Pen;
using Gdiplus::Rect;
using Gdiplus::SolidBrush;

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
    std::unique_ptr<Image> image;
    Rect image_rect;
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
HWND g_reset = nullptr;

constexpr int ID_TEMPLATES = 1001;
constexpr int ID_PARTS = 1002;
constexpr int ID_OPEN = 1003;
constexpr int ID_RESET = 1004;
constexpr int ID_DEV = 1005;
constexpr int ID_APPLY = 1006;
constexpr int ID_NEW = 1007;
constexpr int ID_DELETE = 1008;
constexpr int ID_SAVE = 1009;

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

int get_control_int(HWND hwnd, int fallback) {
    try {
        return std::stoi(read_control(hwnd));
    } catch (...) {
        return fallback;
    }
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

void load_image(const fs::path& path) {
    g_app.current_image_path = path;
    g_app.image.reset(Image::FromFile(path.wstring().c_str()));
    if (!g_app.image || g_app.image->GetLastStatus() != Gdiplus::Ok) {
        g_app.image.reset();
        set_status(L"Falha ao carregar imagem.");
        return;
    }
    set_status(L"Imagem carregada: " + path.filename().wstring());
    InvalidateRect(g_main, nullptr, FALSE);
}

void load_default_image() {
    TextureTemplate* texture = current_template();
    if (!texture) {
        return;
    }
    fs::path path = g_app.root / texture->default_image;
    load_image(path);
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
    int left = 250;
    int top = 56;
    int status_h = 28;
    int dev = g_app.dev_mode ? 285 : 0;
    int gap = 10;

    MoveWindow(g_open, 12, 12, 110, 30, TRUE);
    MoveWindow(g_reset, 130, 12, 120, 30, TRUE);
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

    HWND children[] = {g_id, g_label, g_x, g_y, g_w, g_h, g_apply, g_new_part, g_delete_part, g_save};
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
        for (HWND label : labels) {
            ShowWindow(label, SW_SHOW);
        }
        place(labels[0], g_id);
        place(labels[1], g_label);
        place(labels[2], g_x);
        place(labels[3], g_y);
        place(labels[4], g_w);
        place(labels[5], g_h);
        MoveWindow(g_apply, right_x, y + gap, dev - 24, 30, TRUE);
        MoveWindow(g_new_part, right_x, y + 46, dev - 24, 30, TRUE);
        MoveWindow(g_delete_part, right_x, y + 82, dev - 24, 30, TRUE);
        MoveWindow(g_save, right_x, y + 128, dev - 24, 34, TRUE);
    } else {
        for (int id = 2001; id <= 2006; ++id) {
            ShowWindow(GetDlgItem(hwnd, id), SW_HIDE);
        }
    }
    InvalidateRect(hwnd, nullptr, TRUE);
}

Rect canvas_rect(HWND hwnd) {
    RECT client{};
    GetClientRect(hwnd, &client);
    int left = 260;
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
    double scale = std::min(bounds.Width / iw, bounds.Height / ih);
    int w = static_cast<int>(iw * scale);
    int h = static_cast<int>(ih * scale);
    return Rect(bounds.X + (bounds.Width - w) / 2, bounds.Y + (bounds.Height - h) / 2, w, h);
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

void paint(HWND hwnd) {
    PAINTSTRUCT ps{};
    HDC hdc = BeginPaint(hwnd, &ps);
    Graphics g(hdc);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.Clear(Color(255, 18, 22, 29));

    SolidBrush top(Color(255, 27, 33, 43));
    g.FillRectangle(&top, 0, 0, ps.rcPaint.right, 56);

    SolidBrush panel(Color(255, 24, 29, 38));
    g.FillRectangle(&panel, 0, 56, 250, ps.rcPaint.bottom);
    if (g_app.dev_mode) {
        g.FillRectangle(&panel, ps.rcPaint.right - 295, 56, 295, ps.rcPaint.bottom);
    }

    Rect canvas = canvas_rect(hwnd);
    SolidBrush canvas_brush(Color(255, 12, 14, 18));
    g.FillRectangle(&canvas_brush, canvas);

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
            g.DrawRectangle(&cyan, highlight);

            int preview_w = std::min(430, std::max(180, canvas.Width / 3));
            int preview_h = std::min(260, std::max(140, canvas.Height / 3));
            Rect preview(canvas.X + canvas.Width - preview_w - 18, canvas.Y + canvas.Height - preview_h - 18, preview_w, preview_h);
            SolidBrush preview_bg(Color(230, 20, 25, 33));
            g.FillRectangle(&preview_bg, preview);
            g.DrawRectangle(&cyan, preview);
            Rect src(part->x, part->y, part->w, part->h);
            g.DrawImage(g_app.image.get(), preview, src.X, src.Y, src.Width, src.Height, Gdiplus::UnitPixel);
        }
    }

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
    return CreateWindowExW(0, cls, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 10, 10, parent, reinterpret_cast<HMENU>(id), GetModuleHandleW(nullptr), nullptr);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE:
        g_main = hwnd;
        g_open = make_child(hwnd, L"BUTTON", L"Abrir PNG", BS_PUSHBUTTON, ID_OPEN);
        g_reset = make_child(hwnd, L"BUTTON", L"Template padrao", BS_PUSHBUTTON, ID_RESET);
        g_dev_check = make_child(hwnd, L"BUTTON", L"Modo-dev", BS_AUTOCHECKBOX, ID_DEV);
        SendMessageW(g_dev_check, BM_SETCHECK, BST_CHECKED, 0);
        g_templates = make_child(hwnd, L"LISTBOX", L"", LBS_NOTIFY | WS_BORDER | WS_VSCROLL, ID_TEMPLATES);
        g_parts = make_child(hwnd, L"LISTBOX", L"", LBS_NOTIFY | WS_BORDER | WS_VSCROLL, ID_PARTS);
        g_status = make_child(hwnd, L"STATIC", L"Pronto.", SS_LEFT, 0);

        make_child(hwnd, L"STATIC", L"ID", SS_LEFT, 2001);
        make_child(hwnd, L"STATIC", L"Label", SS_LEFT, 2002);
        make_child(hwnd, L"STATIC", L"X", SS_LEFT, 2003);
        make_child(hwnd, L"STATIC", L"Y", SS_LEFT, 2004);
        make_child(hwnd, L"STATIC", L"W", SS_LEFT, 2005);
        make_child(hwnd, L"STATIC", L"H", SS_LEFT, 2006);
        g_id = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 0);
        g_label = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_AUTOHSCROLL, 0);
        g_x = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_y = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_w = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_h = make_child(hwnd, L"EDIT", L"", WS_BORDER | ES_NUMBER, 0);
        g_apply = make_child(hwnd, L"BUTTON", L"Aplicar parte", BS_PUSHBUTTON, ID_APPLY);
        g_new_part = make_child(hwnd, L"BUTTON", L"Nova parte", BS_PUSHBUTTON, ID_NEW);
        g_delete_part = make_child(hwnd, L"BUTTON", L"Remover parte", BS_PUSHBUTTON, ID_DELETE);
        g_save = make_child(hwnd, L"BUTTON", L"Salvar JSON", BS_PUSHBUTTON, ID_SAVE);

        rebuild_templates();
        load_default_image();
        layout(hwnd);
        return 0;
    case WM_SIZE:
        layout(hwnd);
        return 0;
    case WM_LBUTTONDOWN:
        select_part_from_point(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        return 0;
    case WM_COMMAND:
        switch (LOWORD(wparam)) {
        case ID_TEMPLATES:
            if (HIWORD(wparam) == LBN_SELCHANGE) {
                g_app.selected_template = static_cast<int>(SendMessageW(g_templates, LB_GETCURSEL, 0, 0));
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
        case ID_RESET:
            load_default_image();
            return 0;
        case ID_DEV:
            g_app.dev_mode = SendMessageW(g_dev_check, BM_GETCHECK, 0, 0) == BST_CHECKED;
            layout(hwnd);
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
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1280,
        780,
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

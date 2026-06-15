#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

struct Options {
    std::string template_id;
    std::string part_id;
    std::string input;
    std::string output;
};

std::string read_file(const fs::path& path) {
    std::ifstream file(path);
    if (!file) {
        throw std::runtime_error("Nao foi possivel abrir " + path.string());
    }
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

fs::path repo_root() {
    fs::path current = fs::current_path();
    while (!current.empty()) {
        if (fs::exists(current / "data" / "templates" / "teeworlds_textures.json")) {
            return current;
        }
        current = current.parent_path();
    }
    return fs::current_path();
}

void print_help() {
    std::cout
        << "Teeworlds Texture Editor " << TTE_VERSION << "\n\n"
        << "Comandos:\n"
        << "  tte list\n"
        << "  tte parts <template>\n"
        << "  tte focus --template <id> --part <id> [--input <png>] --output <png>\n";
}

void list_templates(const std::string& json) {
    const std::regex template_regex(R"REGEX("id"\s*:\s*"([^"]+)"\s*,\s*"name"\s*:\s*"([^"]+)")REGEX");
    auto begin = std::sregex_iterator(json.begin(), json.end(), template_regex);
    auto end = std::sregex_iterator();

    std::cout << "Templates disponiveis:\n";
    for (auto it = begin; it != end; ++it) {
        std::cout << "  " << (*it)[1].str() << " - " << (*it)[2].str() << "\n";
    }
}

std::string extract_template_block(const std::string& json, const std::string& template_id) {
    const std::string needle = "\"id\": \"" + template_id + "\"";
    const size_t id_pos = json.find(needle);
    if (id_pos == std::string::npos) {
        throw std::runtime_error("Template nao encontrado: " + template_id);
    }

    const size_t parts_pos = json.find("\"parts\"", id_pos);
    const size_t next_template = json.find("\n    {", parts_pos + 1);
    if (parts_pos == std::string::npos) {
        throw std::runtime_error("Template sem lista de partes: " + template_id);
    }

    return json.substr(parts_pos, next_template == std::string::npos ? std::string::npos : next_template - parts_pos);
}

void list_parts(const std::string& json, const std::string& template_id) {
    const std::string block = extract_template_block(json, template_id);
    const std::regex part_regex(R"REGEX(\{\s*"id"\s*:\s*"([^"]+)"\s*,\s*"label"\s*:\s*"([^"]+)")REGEX");
    auto begin = std::sregex_iterator(block.begin(), block.end(), part_regex);
    auto end = std::sregex_iterator();

    std::cout << "Partes de '" << template_id << "':\n";
    for (auto it = begin; it != end; ++it) {
        std::cout << "  " << (*it)[1].str() << " - " << (*it)[2].str() << "\n";
    }
}

std::string quote(const std::string& value) {
    std::string escaped = value;
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != std::string::npos) {
        escaped.insert(pos, "\\");
        pos += 2;
    }
    return "\"" + escaped + "\"";
}

Options parse_focus_options(int argc, char** argv) {
    Options options;
    for (int i = 2; i < argc; ++i) {
        const std::string key = argv[i];
        if (i + 1 >= argc) {
            throw std::runtime_error("Opcao sem valor: " + key);
        }
        const std::string value = argv[++i];
        if (key == "--template") {
            options.template_id = value;
        } else if (key == "--part") {
            options.part_id = value;
        } else if (key == "--input") {
            options.input = value;
        } else if (key == "--output") {
            options.output = value;
        } else {
            throw std::runtime_error("Opcao desconhecida: " + key);
        }
    }

    if (options.template_id.empty() || options.part_id.empty() || options.output.empty()) {
        throw std::runtime_error("focus precisa de --template, --part e --output. --input e opcional.");
    }
    return options;
}

int run_focus(const Options& options) {
    const fs::path root = repo_root();
    const fs::path script = root / "scripts" / "focus_texture.py";

    std::ostringstream command;
    command << "py "
            << quote(script.string())
            << " --template " << quote(options.template_id)
            << " --part " << quote(options.part_id)
            << " --output " << quote(options.output);
    if (!options.input.empty()) {
        command << " --input " << quote(options.input);
    }

    return std::system(command.str().c_str());
}

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            print_help();
            return 0;
        }

        const fs::path templates = repo_root() / "data" / "templates" / "teeworlds_textures.json";
        const std::string json = read_file(templates);
        const std::string command = argv[1];

        if (command == "list") {
            list_templates(json);
            return 0;
        }

        if (command == "parts") {
            if (argc < 3) {
                throw std::runtime_error("Use: tte parts <template>");
            }
            list_parts(json, argv[2]);
            return 0;
        }

        if (command == "focus") {
            return run_focus(parse_focus_options(argc, argv));
        }

        print_help();
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "Erro: " << error.what() << "\n";
        return 1;
    }
}

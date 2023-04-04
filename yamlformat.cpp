#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <yaml-cpp/yaml.h>
#include <reproc++/run.hpp>
#include <rang.hpp>

namespace fs = std::filesystem;

void printNode(const YAML::Node& node, int indentLevel = 0) {
    if (node.IsMap()) {
        for (const auto& keyValue : node) {
            for (int i = 0; i < indentLevel; ++i) {
                std::cout << "  ";
            }
            std::cout << keyValue.first.as<std::string>() << ":\n";
            printNode(keyValue.second, indentLevel + 1);
        }
    } else if (node.IsSequence()) {
        for (const auto& value : node) {
            for (int i = 0; i < indentLevel; ++i) {
                std::cout << "  ";
            }
            std::cout << "- ";
            printNode(value, 0);
        }
    } else {
        std::cout << node.as<std::string>() << '\n';
    }
}

std::string readFile(const std::string& filePath) {
    std::ifstream fileStream(filePath);
    std::string fileContent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
    return fileContent;
}

int main() {
    try {
        for (const auto& entry : fs::directory_iterator(fs::current_path())) {
            if (entry.path().extension() == ".yaml" || entry.path().extension() == ".yml") {
                std::cout << rang::fg::cyan << "Processing file: " << entry.path().filename() << rang::fg::reset << "\n\n";
                YAML::Node yamlData = YAML::LoadFile(entry.path());
                printNode(yamlData);

                std::string originalContent = readFile(entry.path());
                std::ostringstream formattedContentStream;
                formattedContentStream << yamlData;
                std::string formattedContent = formattedContentStream.str();

                if (originalContent != formattedContent) {
                    std::cout << rang::fg::yellow << "Differences found. Here are the changes to correct the YAML syntax:\n" << rang::fg::reset;
                    std::string diffCommand = "diff --color=always --unified <(echo \"" + originalContent + "\") <(echo \"" + formattedContent + "\")";
                    reproc::run_options options;
                    options.redirect.error.type = reproc::redirect::parent;
                    options.redirect.output.type = reproc::redirect::parent;
                    options.redirect.input.type = reproc::redirect::none;
                    options.shell = true;
                    reproc::run(diffCommand, options);
                }

                std::cout << "\n";
            }
        }

    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error: Unable to access the directory: " << e.what() << '\n';
        return 1;
    } catch (const YAML::BadFile& e) {
        std::cerr << "Error: Unable to open YAML file: " << e.what() << '\n';
        return 1;
    } catch (const YAML::ParserException& e) {
        std::cerr << "Error: Invalid YAML syntax: " << e.what() << '\n';
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}

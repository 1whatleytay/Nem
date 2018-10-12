//
// Created by Taylor Whatley on 2018-09-18.
//

#ifndef NEM_ERROR_H
#define NEM_ERROR_H

#include <string>
#include <exception>

namespace Nem {
    class CouldNotCreateWindowException: public std::exception {
    public:
        const char* what() const noexcept override {
            return "Could not create window.";
        }
    };

    class RomNotFoundException: public std::exception {
        std::string path;
    public:
        const char* what() const noexcept override {
            return ("Could not find ROM at " + path).c_str();
        }

        explicit RomNotFoundException(std::string nPath) : path(nPath) { }
    };

    class ShaderNotFoundException: public std::exception {
        std::string path;
    public:
        const char* what() const noexcept override {
            return ("Could not find shader at " + path).c_str();
        }

        explicit ShaderNotFoundException(std::string nPath) : path(nPath) { }
    };

    class RomInvalidException: public std::exception {
        std::string romIssue;
    public:
        const char* what() const noexcept override {
            return ("Rom is " + romIssue).c_str();
        }

        explicit RomInvalidException(std::string nRomIssue) : romIssue(nRomIssue) { }
    };

    class RomUnimplementedException: public std::exception {
        std::string issue;
    public:
        const char* what() const noexcept override {
            return ("Rom is using unimplemented " + issue).c_str();
        }

        explicit RomUnimplementedException(std::string nIssue) : issue(nIssue) { }
    };
}

#endif //NEM_ERROR_H

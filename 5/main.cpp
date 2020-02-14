#include <iostream>
#include <fstream>
#include <map>
#include <functional>

#include "core/fs.h"
#include "utils/utils.h"

const string &welcomeMessage = R"(
                        ___         ___
         _____         /  /\       /  /\
        /  /::\       /  /:/_     /  /:/_
       /  /:/\:\     /  /:/ /\   /  /:/ /\
      /  /:/~/::\   /  /:/ /:/  /  /:/ /::\
     /__/:/ /:/\:| /__/:/ /:/  /__/:/ /:/\:\
     \  \:\/:/~/:/ \  \:\/:/   \  \:\/:/~/:/
      \  \::/ /:/   \  \::/     \  \::/ /:/
       \  \:\/:/     \  \:\      \__\/ /:/
        \  \::/       \  \:\       /__/:/
         \__\/         \__\/       \__\/

    Welcome to BFS!
)";

void store(FileSystem &fs, const string &filename, const string &path) {
    ofstream stream(path);
    if (stream.fail()) {
        throw runtime_error("Unable to open " + path);
    }
    stream << fs.readFile(filename);
}

void load(FileSystem &fs, const string &path, const string &filename) {
    ifstream stream(path);
    if (stream.fail()) {
        throw runtime_error("Unable to open " + path);
    }
    auto buffer = string(istreambuf_iterator<char>(stream), istreambuf_iterator<char>());
    fs.writeFile(filename, buffer);
}

void printStat(const string &filename, FileSystem::InodeBase inode) {
    cout << Utils::formatSize(inode.size).rdbuf() << " "
         << oct << inode.mode << " "
         << dec << inode.uid << " "
         << Utils::formatTimePoint(inode.create_time) << " "
         << Utils::formatTimePoint(inode.modification_time) << " "
         << filename << endl;
}

void printHelp() {
    cout << "Commands:" << endl
         << "    format" << endl
         << "    mount" << endl
         << "    cat <file>" << endl
         << "    store <file> <file_outside_bfs>" << endl
         << "    load <file_outside_bfs> <file>" << endl
         << "    touch <file>" << endl
         << "    mkdir <directory>" << endl
         << "    ls [directory]" << endl
         << "    stat <file>" << endl
         << "    rm <file>" << endl
         << "    write <file> <data>" << endl
         << "    mv <from> <to>" << endl
         << "    cp <from> <to>" << endl
         << "    help" << endl
         << "    exit" << endl;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <diskFilePath>" << endl;
        return EXIT_FAILURE;
    }

    Disk disk(argv[1]);
    FileSystem fs(disk);

    map<string, function<void(const string &, const string &)>> funcs = {
        {"format", [&fs](const string &, const string &) {
            fs.format();
        }},
        {"mount",  [&fs](const string &, const string &) {
            fs.mount();
        }},
        {"cat",    [&fs](const string &arg1, const string &) {
            if (arg1.empty())
                throw runtime_error("Usage: cat <file>");
            store(fs, arg1, "/dev/stdout");
            cout << endl;
        }},
        {"store",  [&fs](const string &arg1, const string &arg2) {
            if (arg1.empty() || arg2.empty())
                throw runtime_error("Usage: store <file> <file_outside_bfs>");
            store(fs, arg1, arg2);
        }},
        {"load",   [&fs](const string &arg1, const string &arg2) {
            if (arg1.empty() || arg2.empty())
                throw runtime_error("Usage: load <file_outside_bfs> <file>");
            load(fs, arg1, arg2);
        }},
        {"touch",  [&fs](const string &arg1, const string &) {
            if (arg1.empty())
                throw runtime_error("Usage: touch <file>");
            if (arg1[arg1.length() - 1] == '/')
                throw runtime_error("Touching directory is not allowed. Please use mkdir instead");
            fs.createFile(arg1);
        }},
        {"mkdir",  [&fs](const string &arg1, const string &) {
            if (arg1.empty())
                throw runtime_error("Usage: mkdir <directory>");
            fs.createFile(arg1[arg1.length() - 1] == '/' ? arg1 : arg1 + '/');
        }},
        {"ls",     [&fs](const string &arg1, const string &) {
            for (const auto &[filename, inode]: fs.listDirectory(arg1))
                printStat(filename, inode);
        }},
        {"cd",     [&fs](const string &arg1, const string &) {
            if (arg1.empty())
                throw runtime_error("Usage: cd <directory>");
            fs.changeDirectory(arg1);
        }},
        {"stat",   [&fs](const string &arg1, const string &) {
            if (arg1.empty())
                throw runtime_error("Usage: stat <file>");
            printStat(arg1, fs.statFile(arg1));
        }},
        {"rm",     [&fs](const string &arg1, const string &) {
            if (arg1.empty())
                throw runtime_error("Usage: rm <file>");
            fs.removeFile(arg1);
        }},
        {"write",  [&fs](const string &arg1, const string &arg2) {
            if (arg1.empty() || arg2.empty())
                throw runtime_error("Usage: write <file> <data>");
            fs.writeFile(arg1, arg2);
        }},
        {"mv",     [&fs](const string &arg1, const string &arg2) {
            if (arg1.empty() || arg2.empty())
                throw runtime_error("Usage: mv <from> <to>");
            fs.moveFile(arg1, arg2);
        }},
        {"cp",     [&fs](const string &arg1, const string &arg2) {
            if (arg1.empty() || arg2.empty())
                throw runtime_error("Usage: cp <from> <to>");
            fs.copyFile(arg1, arg2);
        }},
        {"help",   [&fs](const string &, const string &) {
            printHelp();
        }},
        {"exit",   [&fs](const string &, const string &) {
            exit(EXIT_SUCCESS);
        }},
        {"default",   [&fs](const string &arg1, const string &) {
            cout << "Unknown command: " << arg1 << endl << "Type 'help' to get help." << endl;
        }},
    };
    cout << welcomeMessage;
    while (true) {
        try {
            cout << "BFS> ";
            string input, cmd, arg1, arg2;
            if (!getline(cin, input)) {
                break;
            }
            istringstream iss(input);
            iss >> cmd >> arg1 >> arg2;
            funcs.contains(cmd) ? funcs[cmd](arg1, arg2) : funcs["default"](input, "");
        } catch (runtime_error &e) {
            cout << e.what() << endl;
        }
    }
    return EXIT_SUCCESS;
}

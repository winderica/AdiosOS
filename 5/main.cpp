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
    cout << setw(5) << Utils::formatSize(inode.size).str() << " "
         << oct << inode.mode
         << dec << setw(5) << inode.uid << " "
         << Utils::formatTimePoint(inode.creationTime) << " "
         << Utils::formatTimePoint(inode.modificationTime) << " "
         << filename << endl;
}

void printHelp() {
    cout << "Commands:" << endl
         << "    format" << endl
         << "    mount" << endl
         << "    store <file> <file_outside_bfs>" << endl
         << "    load <file_outside_bfs> <file>" << endl
         << "    touch <file>" << endl
         << "    mkdir <directory>" << endl
         << "    cd <directory>" << endl
         << "    ls [directory]" << endl
         << "    stat <file>" << endl
         << "    cat <file>" << endl
         << "    write <file> <data>" << endl
         << "    mv <from> <to>" << endl
         << "    cp <from> <to>" << endl
         << "    rm <file>" << endl
         << "    su <uid>" << endl
         << "    chown <uid> <file>" << endl
         << "    chmod <mode> <file>" << endl
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

    // map of functions is much more elegant than if-else/switch-case
    map<string, function<void(const string &, const string &)>> funcs = {
        {"format",  [&fs](const string &, const string &) {
            fs.format();
        }},
        {"mount",   [&fs](const string &, const string &) {
            fs.mount();
        }},
        {"su",      [&fs](const string &uid, const string &) {
            if (uid.empty())
                throw runtime_error("Usage: su <uid>");
            fs.setUid(stoi(uid));
        }},
        {"chown",   [&fs](const string &uid, const string &file) {
            if (file.empty() || uid.empty())
                throw runtime_error("Usage: chown <uid> <file>");
            fs.changeOwner(file, stoi(uid));
        }},
        {"chmod",   [&fs](const string &mode, const string &file) {
            if (file.empty() || mode.empty())
                throw runtime_error("Usage: chmod <mode> <file>");
            Permissions castMode;
            try {
                castMode = static_cast<Permissions>(stoi(mode, nullptr, 8));
            } catch (...) {
                throw runtime_error("Mode should be in octet");
            }
            fs.changeMode(file, castMode);
        }},
        {"cat",     [&fs](const string &file, const string &) {
            if (file.empty())
                throw runtime_error("Usage: cat <file>");
            store(fs, file, "/dev/stdout");
            cout << endl;
        }},
        {"store",   [&fs](const string &from, const string &to) {
            if (from.empty() || to.empty())
                throw runtime_error("Usage: store <file> <file_outside_bfs>");
            store(fs, from, to);
        }},
        {"load",    [&fs](const string &from, const string &to) {
            if (from.empty() || to.empty())
                throw runtime_error("Usage: load <file_outside_bfs> <file>");
            load(fs, from, to);
        }},
        {"touch",   [&fs](const string &file, const string &) {
            if (file.empty())
                throw runtime_error("Usage: touch <file>");
            if (file[file.length() - 1] == '/')
                throw runtime_error("Touching directory is not allowed. Please use mkdir instead");
            fs.createFile(file);
        }},
        {"mkdir",   [&fs](const string &directory, const string &) {
            if (directory.empty())
                throw runtime_error("Usage: mkdir <directory>");
            fs.createFile(directory[directory.length() - 1] == '/' ? directory : directory + '/');
        }},
        {"ls",      [&fs](const string &directory, const string &) {
            for (const auto &[filename, inode]: fs.listDirectory(directory))
                printStat(filename, inode);
        }},
        {"cd",      [&fs](const string &directory, const string &) {
            if (directory.empty())
                throw runtime_error("Usage: cd <directory>");
            fs.changeDirectory(directory);
        }},
        {"stat",    [&fs](const string &file, const string &) {
            if (file.empty())
                throw runtime_error("Usage: stat <file>");
            printStat(file, fs.statFile(file));
        }},
        {"rm",      [&fs](const string &file, const string &) {
            if (file.empty())
                throw runtime_error("Usage: rm <file>");
            fs.removeFile(file);
        }},
        {"write",   [&fs](const string &file, const string &data) {
            if (file.empty() || data.empty())
                throw runtime_error("Usage: write <file> <data>");
            fs.writeFile(file, data);
        }},
        {"mv",      [&fs](const string &from, const string &to) {
            if (from.empty() || to.empty())
                throw runtime_error("Usage: mv <from> <to>");
            fs.moveFile(from, to);
        }},
        {"cp",      [&fs](const string &from, const string &to) {
            if (from.empty() || to.empty())
                throw runtime_error("Usage: cp <from> <to>");
            fs.copyFile(from, to);
        }},
        {"help",    [&fs](const string &, const string &) {
            printHelp();
        }},
        {"exit",    [&fs](const string &, const string &) {
            exit(EXIT_SUCCESS);
        }},
        {"default", [&fs](const string &cmd, const string &) {
            cout << "Unknown command: " << cmd << endl << "Type 'help' to get help." << endl;
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
            iss >> cmd >> arg1 >> ws; // consume whitespace
            getline(iss, arg2); // arg2 is the remainder
            funcs.contains(cmd) ? funcs[cmd](arg1, arg2) : funcs["default"](input, "");
        } catch (runtime_error &e) {
            cout << e.what() << endl;
        } catch (invalid_argument &e) {
            cout << "Invalid Argument: " << e.what() << endl;
        } catch (...) {
            cout << "Unexpected Error" << endl;
        }
    }
    return EXIT_SUCCESS;
}

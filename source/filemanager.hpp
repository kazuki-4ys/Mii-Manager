#include <dirent.h>
#include <string>
#include <cstring>
#include <vector>

using namespace std;

typedef struct {
    string name;
    bool isDir;
} entry;

class FileManager{
    public:
        bool valid;
        string curPath;
        vector<entry> entries;
        FileManager(string);
        int OpenPath(string);
        int Reload(void);
        int Back(void);
        string getFullPath(int);
};
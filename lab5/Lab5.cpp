#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <dirent.h>
#include <vector>
#include <iomanip>

using namespace std;

int lvl = 0;

void file_attributes(struct stat buf, string filename)
{
    struct passwd *pwd;
    struct group *grp;

    pwd = getpwuid(buf.st_uid);
    if (pwd == NULL) {
        perror("Owner ID error");
    }

    grp = getgrgid(buf.st_gid);
    if (grp == NULL) {
        perror("Group ID error");
    }

    time_t mt = buf.st_mtim.tv_sec;
    string last_time = ctime(&mt);
    last_time[last_time.length() - 1] = '\0';

    cout << oct << "MODE: " << setw(6) << buf.st_mode << " | ";
    cout << "LINKS: " << setw(2) << buf.st_nlink << " | ";
    cout << "OWNER: " << pwd->pw_name << " | ";
    cout << "GROUP: " << grp->gr_name << " | ";
    cout << dec << "SIZE (bytes): " << setw(6) << buf.st_size << " | ";
    cout << "# OF BLOCKS (512B): " << setw(3) << buf.st_blocks << " | ";
    cout << "LAST MODIFIED: " << last_time << "        ";
    cout << filename << endl;

}

void dir_traversal(string path)
{
    DIR *dir;
    struct stat buf;
    if ( (dir = opendir(path.c_str())) == NULL ) {
        string error_msg = "Cannot open " + path; 
        perror(error_msg.c_str());
    }

    struct dirent *sd;
    int num_spaces = (lvl == 0) ? 0 : lvl*3;
    string spaces = string(num_spaces, ' ');
    if (dir) {
        cout << "\n" << path << ":" << endl;
        vector<string> dirs_list;
        while((sd = readdir(dir)) != NULL) {
            string subPath = "";
            if (strcmp(sd->d_name, ".") == 0 || strcmp(sd->d_name, "..") == 0)
                continue;
            
            if (path[path.length()-1] == '/')
                subPath = path + sd->d_name;
            else
                subPath = path + "/" + sd->d_name;
            
            if (stat(subPath.c_str(), &buf) == -1) {
                perror("stat");
            }
            else {
                file_attributes(buf, sd->d_name);
                if (sd->d_type == DT_DIR) {
                    dirs_list.push_back(subPath);
                }
            }
        }
        while(!dirs_list.empty()) {
            dir_traversal(dirs_list.back());
            dirs_list.pop_back();
        }
    }

    closedir(dir);
}

int main(int argc, char *argv[])
{
    struct stat buf;
    string path;

    if (argc != 2) {
        cerr << "Missing argument\n";
    }

    if (stat(argv[1], &buf) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    else {
        path = argv[1];
    }

    
    if (S_ISREG(buf.st_mode)) {                                     // check if file or directory
        file_attributes(buf, path);   
    }
    else if (S_ISDIR(buf.st_mode)) {
        dir_traversal(path);
    }
    

    exit(EXIT_SUCCESS);
}
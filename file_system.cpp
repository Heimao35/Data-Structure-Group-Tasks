#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

// 文件类型枚举
enum FileType {
    DIRECTORY,
    TEXT,
    IMAGE,
    VIDEO,
    AUDIO,
    OTHER
};

// 根据扩展名获取文件类型
FileType getFileTypeByExtension(const string& name) {
    if (name.find('.') == string::npos) {
        return DIRECTORY;
    }
    
    string ext = name.substr(name.find_last_of('.') + 1);
    transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    if (ext == "txt" || ext == "doc" || ext == "pdf") return TEXT;
    if (ext == "jpg" || ext == "png" || ext == "gif") return IMAGE;
    if (ext == "mp4" || ext == "avi" || ext == "mov") return VIDEO;
    if (ext == "mp3" || ext == "wav" || ext == "flac") return AUDIO;
    return OTHER;
}

// 文件节点类，表示文件系统中的文件或目录
class FileNode {
public:
    string name;           // 文件或目录名
    FileType type;         // 文件类型
    int size;              // 文件大小（KB）
    FileNode* left;        // 左子节点
    FileNode* right;       // 右子节点
    FileNode* parent;      // 父节点
    
    // 构造函数
    FileNode(const string& name, FileType type = DIRECTORY, int size = 0) 
        : name(name), type(type), size(size), left(nullptr), right(nullptr), parent(nullptr) {}
    
    // 判断是否为目录
    bool isDirectory() const {
        return type == DIRECTORY;
    }
    
    // 获取文件类型的字符串表示
    string getTypeString() const {
        switch(type) {
            case DIRECTORY: return "目录";
            case TEXT: return "文本";
            case IMAGE: return "图像";
            case VIDEO: return "视频";
            case AUDIO: return "音频";
            case OTHER: return "其他";
        }
        return "未知";
    }
};

// 文件系统类
class FileSystem {
private:
    FileNode* root;    // 根目录
    FileNode* current; // 当前目录
    
    // 查找当前目录下的文件或子目录
    FileNode* findInCurrentDir(const string& name) {
        if (!current) return nullptr;
        
        FileNode* temp = current->left; // 第一个子节点
        while (temp) {
            if (temp->name == name) {
                return temp;
            }
            temp = temp->right; // 下一个兄弟节点
        }
        return nullptr;
    }
    
    // 递归删除节点及其所有子节点
    void deleteNode(FileNode* node) {
        if (!node) return;
        
        // 首先删除所有子节点
        FileNode* child = node->left;
        while (child) {
            FileNode* next = child->right;
            deleteNode(child);
            child = next;
        }
        
        // 删除当前节点
        delete node;
    }

public:
    // 构造函数
    FileSystem() {
        // 创建根目录
        root = new FileNode("root");
        current = root;
    }
    
    // 析构函数
    ~FileSystem() {
        // 递归删除所有节点
        deleteNode(root);
    }
    
    // 获取当前目录
    FileNode* getCurrentDirectory() {
        return current;
    }
    
    // 创建文件或目录
    bool create(const string& name, bool isDirectory = false, int size = 0) {
        // 检查名称是否已存在
        if (findInCurrentDir(name)) {
            cout << "错误: " << name << " 已存在" << endl;
            return false;
        }
        
        // 创建新节点
        FileType type = isDirectory ? DIRECTORY : getFileTypeByExtension(name);
        FileNode* newNode = new FileNode(name, type, size);
        newNode->parent = current;
        
        // 将新节点添加为当前目录的子节点
        if (!current->left) {
            // 如果当前目录没有子节点，则设为第一个子节点
            current->left = newNode;
        } else {
            // 否则，将其添加到子节点链表的末尾
            FileNode* temp = current->left;
            while (temp->right) {
                temp = temp->right;
            }
            temp->right = newNode;
        }
        
        return true;
    }
    
    // 删除文件或目录
    bool remove(const string& name) {
        // 查找要删除的节点
        FileNode* toDelete = findInCurrentDir(name);
        if (!toDelete) {
            cout << "错误: " << name << " 不存在" << endl;
            return false;
        }
        
        // 从父节点的子节点链表中移除
        if (current->left == toDelete) {
            // 如果是第一个子节点
            current->left = toDelete->right;
        } else {
            // 否则，查找并更新链表
            FileNode* temp = current->left;
            while (temp && temp->right != toDelete) {
                temp = temp->right;
            }
            if (temp) {
                temp->right = toDelete->right;
            }
        }
        
        // 递归删除节点及其所有子节点
        deleteNode(toDelete);
        return true;
    }
    
    // 重命名文件或目录
    bool rename(const string& oldName, const string& newName) {
        // 检查旧名称是否存在
        FileNode* target = findInCurrentDir(oldName);
        if (!target) {
            cout << "错误: " << oldName << " 不存在" << endl;
            return false;
        }
        
        // 检查新名称是否已存在
        if (findInCurrentDir(newName)) {
            cout << "错误: " << newName << " 已存在" << endl;
            return false;
        }
        
        // 更新文件类型(如果扩展名变化)
        if (!target->isDirectory()) {
            target->type = getFileTypeByExtension(newName);
        }
        
        // 更新名称
        target->name = newName;
        return true;
    }
    
    // 进入子目录
    bool enterDirectory(const string& name) {
        FileNode* dir = findInCurrentDir(name);
        if (!dir) {
            cout << "错误: " << name << " 不存在" << endl;
            return false;
        }
        
        if (!dir->isDirectory()) {
            cout << "错误: " << name << " 不是目录" << endl;
            return false;
        }
        
        current = dir;
        return true;
    }
    
    // 返回上级目录
    bool goUp() {
        if (current == root) {
            cout << "已经在根目录" << endl;
            return false;
        }
        
        current = current->parent;
        return true;
    }
    
    // 获取当前路径
    string getCurrentPath() {
        vector<string> path;
        FileNode* temp = current;
        
        // 从当前节点向上遍历
        while (temp) {
            path.push_back(temp->name);
            temp = temp->parent;
        }
        
        // 反转路径并构建字符串
        string result = "";
        for (int i = path.size() - 1; i >= 0; i--) {
            result += "/" + path[i];
        }
        return result.empty() ? "/" : result;
    }
    
    // 前序遍历搜索文件(根-左-右)
    void searchByName(FileNode* node, const string& keyword, vector<FileNode*>& results) {
        if (!node) return;
        
        // 检查当前节点
        if (node->name.find(keyword) != string::npos) {
            results.push_back(node);
        }
        
        // 搜索子节点
        FileNode* child = node->left;
        while (child) {
            searchByName(child, keyword, results);
            child = child->right;
        }
    }
    
    // 中序遍历按文件类型分类(左-根-右)
    void classifyByType(FileNode* node, vector<FileNode*> types[6]) {
        if (!node) return;
        
        // 先处理子节点
        FileNode* child = node->left;
        while (child) {
            classifyByType(child, types);
            child = child->right;
        }
        
        // 处理当前节点
        types[node->type].push_back(node);
    }
    
    // 后序遍历计算目录大小(左-右-根)
    int calculateSize(FileNode* node) {
        if (!node) return 0;
        
        // 如果不是目录，直接返回自身大小
        if (!node->isDirectory()) {
            return node->size;
        }
        
        // 计算所有子节点的总大小
        int totalSize = 0;
        FileNode* child = node->left;
        while (child) {
            totalSize += calculateSize(child);
            child = child->right;
        }
        
        // 更新目录大小并返回
        node->size = totalSize;
        return totalSize;
    }
    
    // 显示当前目录内容
    void listCurrentDirectory() {
        cout << "当前路径: " << getCurrentPath() << endl;
        cout << setw(20) << left << "名称" 
             << setw(10) << left << "类型" 
             << setw(10) << left << "大小(KB)" << endl;
        cout << string(40, '-') << endl;
        
        if (!current->left) {
            cout << "目录为空" << endl;
            return;
        }
        
        FileNode* temp = current->left;
        while (temp) {
            cout << setw(20) << left << temp->name 
                 << setw(10) << left << temp->getTypeString() 
                 << setw(10) << left << temp->size << endl;
            temp = temp->right;
        }
    }
    
    // 按名称搜索文件
    void searchFile(const string& keyword) {
        vector<FileNode*> results;
        searchByName(root, keyword, results);
        
        cout << "搜索结果 (关键词: " << keyword << ")" << endl;
        cout << setw(40) << left << "路径" 
             << setw(10) << left << "类型" 
             << setw(10) << left << "大小(KB)" << endl;
        cout << string(60, '-') << endl;
        
        for (FileNode* node : results) {
            // 构建节点路径
            vector<string> path;
            FileNode* temp = node;
            while (temp && temp != root) {
                path.push_back(temp->name);
                temp = temp->parent;
            }
            
            string pathStr = "/root";
            for (int i = path.size() - 2; i >= 0; i--) {
                pathStr += "/" + path[i];
            }
            
            cout << setw(40) << left << pathStr
                 << setw(10) << left << node->getTypeString()
                 << setw(10) << left << node->size << endl;
        }
        
        if (results.empty()) {
            cout << "未找到匹配项" << endl;
        }
    }
    
    // 按文件类型分类并显示
    void showFilesByType() {
        vector<FileNode*> types[6]; // 对应6种文件类型
        classifyByType(root, types);
        
        const char* typeNames[] = {"目录", "文本", "图像", "视频", "音频", "其他"};
        
        for (int i = 0; i < 6; i++) {
            cout << "=== " << typeNames[i] << " 文件 ===" << endl;
            if (types[i].empty()) {
                cout << "无文件" << endl;
            } else {
                cout << setw(40) << left << "路径" 
                     << setw(20) << left << "名称" 
                     << setw(10) << left << "大小(KB)" << endl;
                cout << string(70, '-') << endl;
                
                for (FileNode* node : types[i]) {
                    // 构建节点路径
                    vector<string> path;
                    FileNode* temp = node->parent;
                    while (temp) {
                        path.push_back(temp->name);
                        temp = temp->parent;
                    }
                    
                    string pathStr = "";
                    for (int i = path.size() - 1; i >= 0; i--) {
                        pathStr += "/" + path[i];
                    }
                    
                    cout << setw(40) << left << pathStr
                         << setw(20) << left << node->name
                         << setw(10) << left << node->size << endl;
                }
            }
            cout << endl;
        }
    }
    
    // 统计文件系统信息
    void showStatistics() {
        // 更新所有目录大小
        calculateSize(root);
        
        // 统计各类型文件数量和总大小
        vector<FileNode*> types[6];
        classifyByType(root, types);
        
        int totalFiles = 0;
        int totalDirs = types[0].size();
        int totalSize = 0;
        
        cout << "=== 文件系统统计信息 ===" << endl;
        const char* typeNames[] = {"目录", "文本", "图像", "视频", "音频", "其他"};
        
        for (int i = 0; i < 6; i++) {
            int typeCount = types[i].size();
            int typeSize = 0;
            
            for (FileNode* node : types[i]) {
                typeSize += node->size;
            }
            
            if (i > 0) { // 不是目录
                totalFiles += typeCount;
                totalSize += typeSize;
            }
            
            cout << typeNames[i] << ": " << typeCount << " 个, 共 " 
                 << typeSize << " KB" << endl;
        }
        
        cout << "总文件数: " << totalFiles << endl;
        cout << "总目录数: " << totalDirs << endl;
        cout << "总大小: " << totalSize << " KB" << endl;
    }
};

// 主函数
int main() {
    FileSystem fs;
    string command, param1, param2;
    
    cout << "欢迎使用智能文件管理系统" << endl;
    cout << "输入 'help' 查看可用命令" << endl;
    
    while (true) {
        cout << fs.getCurrentPath() << "> ";
        cin >> command;
        
        if (command == "exit") {
            break;
        } else if (command == "help") {
            cout << "可用命令:" << endl;
            cout << "  ls                      - 显示当前目录内容" << endl;
            cout << "  mkdir [name]            - 创建目录" << endl;
            cout << "  touch [name] [size]     - 创建文件" << endl;
            cout << "  cd [dir]                - 进入目录" << endl;
            cout << "  cd ..                   - 返回上级目录" << endl;
            cout << "  rm [name]               - 删除文件或目录" << endl;
            cout << "  rename [old] [new]      - 重命名文件或目录" << endl;
            cout << "  find [keyword]          - 按名称搜索文件" << endl;
            cout << "  type                    - 按类型显示所有文件" << endl;
            cout << "  stats                   - 显示文件系统统计信息" << endl;
            cout << "  exit                    - 退出程序" << endl;
        } else if (command == "ls") {
            fs.listCurrentDirectory();
        } else if (command == "mkdir") {
            cin >> param1;
            if (fs.create(param1, true)) {
                cout << "目录 " << param1 << " 创建成功" << endl;
            }
        } else if (command == "touch") {
            cin >> param1 >> param2;
            if (fs.create(param1, false, stoi(param2))) {
                cout << "文件 " << param1 << " 创建成功" << endl;
            }
        } else if (command == "cd") {
            cin >> param1;
            if (param1 == "..") {
                fs.goUp();
            } else {
                fs.enterDirectory(param1);
            }
        } else if (command == "rm") {
            cin >> param1;
            if (fs.remove(param1)) {
                cout << param1 << " 已删除" << endl;
            }
        } else if (command == "rename") {
            cin >> param1 >> param2;
            if (fs.rename(param1, param2)) {
                cout << param1 << " 已重命名为 " << param2 << endl;
            }
        } else if (command == "find") {
            cin >> param1;
            fs.searchFile(param1);
        } else if (command == "type") {
            fs.showFilesByType();
        } else if (command == "stats") {
            fs.showStatistics();
        } else {
            cout << "未知命令，输入 'help' 查看可用命令" << endl;
        }
    }
    
    cout << "感谢使用智能文件管理系统！" << endl;
    return 0;
} 
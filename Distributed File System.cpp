#include <iostream>
#include <vector> 
#include<fstream>
#include<WS2tcpip.h>
#include <thread> 
#include<sstream>
#include<mutex>
#include<queue>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

class File
{
private:
	vector<int> pages;
	string fileName;
	string last_modified_by;
	string last_modified_at;
	int ptr;

public:
	File(string fname)
	{
		this->fileName = fname;
		this->ptr = 0;
	}

	void rename(string newName)
	{
		this->fileName = newName;
	}

	void append(int pageno)
	{
		for (int i = 0; i < pages.size(); i++)
			if (this->pages[i] == pageno)
				return;
		pages.push_back(pageno);
	}

	void delPages(int pageno)
	{
		int chunkid = -1;
		for (int i = 0; i < this->pages.size(); i++)
			if (this->pages[i] == pageno)
				chunkid = i;
		this->pages.erase(this->pages.begin() + chunkid);
	}

	void setPtr(int point)
	{
		this->ptr = point;
	}

	void writeFile(fstream& file)
	{
		file << this->fileName << endl;
		file << this->ptr << endl;
		file << this->pages.size() << endl;
		for (auto i = this->pages.begin(); i != this->pages.end(); i++)
			file << *i << endl;
	}

	void readFile(fstream& fin)
	{
		getline(fin, this->fileName);

		string num;
		getline(fin, num);
		this->ptr = stoi(num);

		getline(fin, num);
		int n = stoi(num);

		string pg_num;
		for (int i = 0; i < n; i++)
		{
			getline(fin, pg_num);
			this->pages.push_back(stoi(pg_num));
		}
	}

	vector<int> getPages()
	{
		return this->pages;
	}

	string getName()
	{
		return this->fileName;
	}

	int getEndFilePtr()
	{
		return this->ptr;
	}
};

class Dir
{
private:
	vector<File*> files;
	vector<Dir*> child;
	string name;

public:
	Dir(string dirName)
	{
		this->name = dirName;
	}

	void addFile(File* f)
	{
		this->files.push_back(f);
	}

	void deleteFile(string fileName)
	{
		int index = 0;
		for (auto i = this->files.begin(); i != this->files.end(); i++, index++)
			if ((*i)->getName() == fileName)
			{
				files.erase(this->files.begin() + index);
				return;
			}
	}

	void addSubDir(Dir* dir)
	{

		this->child.push_back(dir);
	}

	void deleteSubDir(string subName)
	{
		int index = 0;
		for (auto i = this->child.begin(); i != this->child.end(); i++, index++)
			if ((*i)->getName() == subName)
			{
				child.erase(this->child.begin() + index);
				return;
			}
	}

	vector<File*> getFiles()
	{
		return this->files;
	}

	vector<Dir*> getChildren()
	{
		return this->child;
	}

	string getName()
	{
		return this->name;
	}
};

class Tree
{
private:
	struct pointer
	{
		Dir* curr;
		string path = "";
		bool available = true;
	};

	Dir* root;
	vector<pointer*> p;
	int n;
public:
	Tree(int n)
	{
		this->n = n;
		this->root = new Dir("root");
		for (int i = 0; i < n; i++)
		{
			pointer* p = new pointer();
			p->path.append("root:\\");
			p->curr = (this->root);
			this->p.push_back(p);
		}
	}

	int createDir(int user, string dirName)
	{
		vector<Dir*> dirs = this->p[user]->curr->getChildren();
		for (int i = 0; i < dirs.size(); i++)
			if (dirs[i]->getName().compare(dirName) == 0)
				return 0;

		Dir* dir = new Dir(dirName);
		this->p[user]->curr->addSubDir(dir);

		return 1;
	}

	void changeDir(int user, string dirName)
	{
		vector<Dir*> children = this->p[user]->curr->getChildren();

		for (auto i = children.begin(); i != children.end(); i++)
			if ((*i)->getName() == dirName)
			{
				this->p[user]->curr = (*i);
				this->p[user]->path.append(this->p[user]->curr->getName() + "\\");
				break;
			}
	}

	void addFile(int user, File* file)
	{
		this->p[user]->curr->addFile(file);
	}

	File* getFile(int user, string fileName)
	{
		vector<File*> files = this->p[user]->curr->getFiles();

		for (auto i = files.begin(); i != files.end(); i++)
			if ((*i)->getName().compare(fileName) == 0)
				return (*i);


		return NULL;
	}

	void deleteFile(int user, string fileName)
	{
		this->p[user]->curr->deleteFile(fileName);
	}

	void writeTree(fstream& fout)
	{
		writetraverse(root, fout);
	}

	void readTree(fstream& fin)
	{
		this->root = readDir(fin);
		readCreateTree(root, fin);
		for (int i = 0; i < this->n; i++)
			this->p[i]->curr = this->root;
	}

	bool fileExists(int user, string fname)
	{
		vector<File*> files = this->p[user]->curr->getFiles();
		for (int i = 0; i < files.size(); i++)
			if (files[i]->getName().compare(fname) == 0)
				return true;

		return false;
	}

	string getPath(int user)
	{
		return this->p[user]->path;
	}

	vector<File*> getCurrFiles(int user)
	{
		return this->p[user]->curr->getFiles();
	}

	string memoryMap()
	{
		ostringstream* ss = new ostringstream();
		levelOrder(root, (*ss), "");
		return ss->str();
	}

	void resetP(int user)
	{
		this->p[user]->path = "root:\\";
		this->p[user]->curr = this->root;
		this->p[user]->available = true;

	}

	int assignFreeP()
	{
		for (int i = 0; i < this->n; i++)
			if (this->p[i]->available)
			{
				this->p[i]->available = false;
				return i;
			}

		return -1;
	}


private:
	void writetraverse(Dir* current, fstream& fout)
	{
		fout << current->getName() << endl;
		fout << current->getFiles().size() << endl;

		vector<File*> files = current->getFiles();

		for (auto file = files.begin(); file != files.end(); file++)
			(*file)->writeFile(fout);

		fout << current->getChildren().size() << endl;
		vector<Dir*> children = current->getChildren();
		for (auto child = children.begin(); child != children.end(); child++)
			writetraverse(*child, fout);
	}

	void readCreateTree(Dir* current, fstream& fin)
	{
		string numChild;
		getline(fin, numChild);
		int n = stoi(numChild);

		for (int i = 0; i < n; i++)
		{
			Dir* child = readDir(fin);
			current->addSubDir(child);
			readCreateTree(child, fin);
		}
	}

	Dir* readDir(fstream& fin)
	{
		string dirName;
		getline(fin, dirName);

		Dir* current = new Dir(dirName);

		string numFiles;
		getline(fin, numFiles);
		int n = stoi(numFiles);

		for (int i = 0; i < n; i++)
		{
			File* file = new File(" ");
			file->readFile(fin);
			current->addFile(file);
		}
		return current;
	}

	void levelOrder(Dir* current, ostringstream& ss, string indent)
	{
		ss << indent << "./" << current->getName() << endl;

		vector<File*> files = current->getFiles();

		for (auto i = files.begin(); i != files.end(); i++)
		{
			ss << indent << indent << (*i)->getName() << " ";
			vector<int> pages = (*i)->getPages();

			for (auto j = pages.begin(); j != pages.end(); j++)
				ss << (*j) << "->";
			ss << "x" << endl;
		}


		vector<Dir*> children = current->getChildren();

		for (auto i = children.begin(); i != children.end(); i++)
			levelOrder(*i, ss, indent + " ");

	}
};

class FileManagement
{
private:
	int TOTAL_PAGES, PAGE_SIZE;
	bool* pagetable;

public:
	FileManagement(int total_pages, int page_size, bool* pagetable)
	{
		this->TOTAL_PAGES = total_pages;
		this->pagetable = pagetable;
		this->PAGE_SIZE = page_size;
	}

	vector<int> write(string txt, fstream& datFile, vector<int> pages, int offset)
	{
		if (offset == 16)
		{
			pages.push_back(nextAvailablePage());
			offset = 0;
		}

		int bytesRemaining = txt.length();
		int bytesToWrite = this->PAGE_SIZE - offset;
		int stringPos = 0;

		if (pages.size() == 0)
			pages.push_back(this->nextAvailablePage());

		while (bytesRemaining > bytesToWrite)
		{
			//seek to page + offset
			int page = pages.at(pages.size() - 1);
			datFile.seekp(page * this->PAGE_SIZE + offset);
			//write upto pagesize - offset
			datFile << txt.substr(stringPos, bytesToWrite);

			//cout << "Page " << page << endl;
			//cout << "Text " << txt.substr(stringPos, bytesToWrite) << endl;

			stringPos += bytesToWrite;
			bytesRemaining -= bytesToWrite;
			pages.push_back(nextAvailablePage());
			offset = 0;
			bytesToWrite = this->PAGE_SIZE;
		}

		int pg = pages.at(pages.size() - 1);
		datFile.seekp(pg * this->PAGE_SIZE + offset);
		datFile << txt.substr(stringPos, bytesRemaining);

		//cout << "Page " << pg << endl;
		//cout << "Text " << txt.substr(stringPos, bytesRemaining) << endl;

		offset += bytesRemaining;
		//cout << "Offset " << offset << endl;

		pages.push_back(-1);
		pages.push_back(offset);

		return pages;
	}

	string read(fstream& datFile, vector<int>pages, int offset)
	{
		stringstream ss;
		char s;

		for (auto i = pages.begin(); i != pages.end(); i++)
		{
			if (*i == pages[pages.size() - 1])
				break;

			datFile.seekg(*i * this->PAGE_SIZE);
			for (int j = 0; j < this->PAGE_SIZE; j++)
			{
				datFile.get(s);
				ss << s;
			}
		}

		datFile.seekg(pages[pages.size() - 1] * this->PAGE_SIZE);
		for (int i = 0; i < offset; i++)
		{
			datFile.get(s);
			ss << s;
		}
		return ss.str();
	}

	vector<int> truncate(int size, vector<int> pages, int offset)
	{
		vector<int> info;
		int max = (pages.size() - 1) * this->PAGE_SIZE + offset;
		if (size >= max)
			return info;

		int count = size / this->PAGE_SIZE;
		int limit = size % this->PAGE_SIZE;
		int totalPages = pages.size();

		if (count + 1 < pages.size())
		{
			for (int i = 0; i < totalPages - count - 1; i++)
			{
				this->pagetable[pages.back()] = false;
				info.push_back(pages.back());
				pages.pop_back();
			}
		}

		offset = limit;
		info.push_back(-1);
		info.push_back(offset);

		return info;
	}

	string readFrom(fstream& datFile, int start, int size, vector<int> pages, int offset)
	{
		int max = (pages.size() - 1) * this->PAGE_SIZE + offset;
		if (start > max || (start + size) > max)
		{
			return "Start + Size exceedes file size!";
		}

		int startingPage = pages[start / this->PAGE_SIZE];
		int endingPage = pages[(start + size) / this->PAGE_SIZE];
		int count = endingPage - startingPage - 1;

		stringstream ss;
		char s;

		datFile.seekg(startingPage * this->PAGE_SIZE + start);

		if (count < 0)
		{
			for (int i = start; i < start + size; i++)
			{
				datFile.get(s);
				ss << s;
			}
		}
		else
		{
			for (int i = start; i < this->PAGE_SIZE - start; i++)
			{
				datFile.get(s);
				ss << s;
			}

			for (int j = 0; j < count; j++)
			{
				datFile.seekg(pages[j] * this->PAGE_SIZE);
				for (int k = 0; k < this->PAGE_SIZE; k++)
				{
					datFile.get(s);
					ss << s;
				}
			}

			datFile.seekg(endingPage * this->PAGE_SIZE);
			for (int l = 0; l < (start + size) % this->PAGE_SIZE; l++)
			{
				datFile.get(s);
				ss << s;
			}
		}

		return ss.str();
	}

	vector<int> writeAt(fstream& datFile, string txt, vector<int> pages, int offset, int pos)
	{
		vector<int> info = pages;
		int max = (pages.size() - 1) * this->PAGE_SIZE + offset;
		int l = txt.length();

		if (pos > max)
			return pages;

		if (l - (max - pos) > 0)
			info = write(txt.substr(max - pos, -1), datFile, pages, offset);

		int startPage = pages[pos / this->PAGE_SIZE];
		int limit = pos % this->PAGE_SIZE;
		datFile.seekp(startPage * this->PAGE_SIZE + limit);
		datFile << txt.substr(0, this->PAGE_SIZE - limit);

		int currentpage = startPage;

		int count = (l - this->PAGE_SIZE + limit) / this->PAGE_SIZE;
		int strStart = this->PAGE_SIZE - limit;
		for (int i = 1; i <= count; i++)
		{
			datFile.seekp(pages[i] * this->PAGE_SIZE);
			datFile << txt.substr(strStart, this->PAGE_SIZE);
			strStart += this->PAGE_SIZE;
			currentpage = pages[i];
		}

		if (l > this->PAGE_SIZE - limit)
		{
			datFile.seekp((currentpage + 1) * this->PAGE_SIZE);
			datFile << txt.substr(strStart, max - pos);
		}

		if (l - (max - pos) < 0)
		{
			info.push_back(-1);
			info.push_back(offset);
		}

		return info;
	}


private:
	int nextAvailablePage()
	{
		for (int i = 0; i < this->TOTAL_PAGES; i++)
		{
			if (!this->pagetable[i])
			{
				this->pagetable[i] = true;
				return i;
			}
		}
		return -1;
	}
};

class FileSystem
{
private:
	const int PAGE_SIZE = 16;
	const int TOTAL_PAGES = 1024;
	bool* Pages;
	const string FILE_NAME = "FileSystem.dat";
	fstream DATFILE;
	Tree* tree;
	FileManagement* filemanager;
	int offset;
	int limit;

public:
	FileSystem(int n)
	{
		struct stat buffer;
		this->offset = 0;
		this->Pages = new bool[this->TOTAL_PAGES];

		for (int i = 0; i < this->TOTAL_PAGES; i++)
			this->Pages[i] = false;

		this->tree = new Tree(n);
		this->filemanager = new FileManagement(this->TOTAL_PAGES, this->PAGE_SIZE, this->Pages);
		this->limit = this->PAGE_SIZE * this->TOTAL_PAGES;

		if (stat(this->FILE_NAME.c_str(), &buffer) == -1)
		{
			this->DATFILE.open(this->FILE_NAME, fstream::out);
			this->DATFILE.close();
			this->DATFILE.open(this->FILE_NAME, fstream::in | fstream::out);
		}

		else
		{
			this->DATFILE.open(this->FILE_NAME, fstream::in | fstream::out);
			this->DATFILE.seekg(this->limit);
			this->readPageTable();
			this->tree->readTree(this->DATFILE);
			this->DATFILE.seekg(0);
		}

	}

	void createDir(int user, string name)
	{
		this->tree->createDir(user, name);
	}

	void changeDir(int user, string name)
	{
		this->tree->changeDir(user, name);
	}

	void createFile(int user, string name, string data = " ")
	{
		if (this->tree->fileExists(user, name)) return;

		if (data.compare(" ") == 0)
		{
			this->tree->addFile(user, new File(name));
			return;
		}


		File* file = new File(name);
		vector<int> info = this->filemanager->write(data, this->DATFILE, file->getPages(), 0);

		auto i = info.begin();
		for (; i != info.end(); i++)
			if (*i != -1)
				file->append(*i);
			else
				break;

		file->setPtr(*(++i));

		this->tree->addFile(user, file);
	}

	void delFile(int user, string name)
	{
		if (!this->tree->fileExists(user, name))
		{
			return;
		}

		vector<int> pgs = this->tree->getFile(user, name)->getPages();

		for (auto i = pgs.begin(); i != pgs.end(); i++)
			this->Pages[*i] = false;

		this->tree->deleteFile(user, name);
	}

	string readFile(int user, string name, int start = 0, int size = 0)
	{
		string output = "";
		if (!this->tree->fileExists(user, name))
		{
			return output;
		}

		File* file = this->tree->getFile(user, name);

		vector<int> pages = file->getPages();
		int offset = file->getEndFilePtr();

		ostringstream ss;
		if (start == 0 && size == 0)
			ss << this->filemanager->read(this->DATFILE, pages, offset) << endl;
		else
			ss << this->filemanager->readFrom(this->DATFILE, start, size, pages, offset) << endl;

		return ss.str();
	}

	void writeToFile(int user, string fname, string data, int at = 0)
	{
		File* file = this->tree->getFile(user, fname);
		vector<int> info;
		if (at == 0)
			info = this->filemanager->write(data, this->DATFILE, file->getPages(), file->getEndFilePtr());
		else
			info = this->filemanager->writeAt(this->DATFILE, data, file->getPages(), file->getEndFilePtr(), at);

		auto i = info.begin();
		for (; i != info.end(); i++)
			if (*i != -1)
				file->append(*i);
			else
				break;

		file->setPtr(*(++i));
	}

	void truncate(int user, string fname, int size)
	{
		File* file = this->tree->getFile(user, fname);
		vector<int> info = this->filemanager->truncate(size, file->getPages(), file->getEndFilePtr());

		auto i = info.begin();
		for (; i != info.end(); i++)
			if (*i != -1)
				file->delPages(*i);
			else
				break;

		file->setPtr(*(++i));
	}

	string showMemoryMap()
	{
		return this->tree->memoryMap();
	}

	string listFiles(int user)
	{
		vector<File*> files = this->tree->getCurrFiles(user);
		stringstream ss;

		for (auto i = files.begin(); i != files.end(); i++)
			ss << (*i)->getName() << endl;

		return ss.str();
	}

	string path(int user)
	{
		return this->tree->getPath(user);
	}

	void reset(int user)
	{
		tree->resetP(user);
	}

	int getFreeP()
	{
		return tree->assignFreeP();
	}

	~FileSystem()
	{
		this->offset = this->limit;
		this->DATFILE.seekp(this->offset);
		this->writePageTable();
		this->tree->writeTree(this->DATFILE);
		this->DATFILE.close();
	}


private:
	void writePageTable()
	{
		int i = 0;
		try {
			for (; i < this->TOTAL_PAGES; i++)
				DATFILE << this->Pages[i] << endl;
		}
		catch (exception e) {
			cout << i;
		}
	}

	void readPageTable()
	{
		for (int i = 0; i < this->TOTAL_PAGES; i++)
		{
			string n;
			getline(this->DATFILE, n);
			this->Pages[i] = stoi(n);
		}
	}

};

class DistributedFileSystem
{
private:
	struct User
	{
		string name;
		int tree_idx;

		//wait for data
		bool waiting = 0;

		//store data for waiting command
		vector<string> cmd_data;
	};
	
	int n;
	FileSystem* f;
	mutex readerlock, writerlock;
	queue<int>readers;
	vector<User*> users;
	
	int getUser(string userName)
	{
		for (int i = 0; i < this->users.size(); i++)
			if (this->users[i]->name.compare(userName) == 0)
				return i;

		return -1;
	}

	void addReader()
	{
		int thread_id = GetCurrentThreadId();

		this->readerlock.lock();

		if (this->readers.empty())
			this->writerlock.lock();
		this->readers.push(thread_id);

		this->readerlock.unlock();
	}

	void removeReader()
	{
		int thread_id = GetCurrentThreadId();

		this->readerlock.lock();

		this->readers.pop();
		if (this->readers.empty())
			this->writerlock.unlock();

		this->readerlock.unlock();
	}

	void addWriter()
	{
		this->writerlock.lock();
	}

	void removeWriter()
	{
		this->writerlock.unlock();
	}

	//split command from client into space delimeted parts 
	vector<string> getSubStrings(string s)
	{
		vector<string> tokens;
		string delimiter = " ";
		size_t pos = 0;
		string token;

		while ((pos = s.find(delimiter)) != string::npos) {
			token = s.substr(0, pos);
			tokens.push_back(token);
			s.erase(0, pos + delimiter.length());
		}
		tokens.push_back(s);
		return tokens;
	}

public:
	DistributedFileSystem(int n)
	{
		this->n = n;
		this->f = new FileSystem(n);
	}

	int addUser(string userName)
	{
		int idx = this->getUser(userName);
		if (idx == -1)
		{
			User* user = new User();
			user->name = userName;
			user->tree_idx = f->getFreeP();
			this->users.push_back(user);
		}
		else
			this->users[idx]->tree_idx = f->getFreeP();

		return this->getUser(userName);
	}

	int removeUser(string userName)
	{
		int idx = this->getUser(userName);
		f->reset(idx);
		return idx;
	}


	//take command execute it and return the response
	string execute(int user, string client_req)
	{
		User* u = this->users[user];

		vector<string> cmd;
		stringstream ss;

		//if command is waiting for data, client_req is actually data and need to be passed to command 
		if (u->waiting)
		{
			u->cmd_data.push_back(client_req);
			cmd = u->cmd_data;
		}
		//client_req is a command and should be split
		else
			cmd = this->getSubStrings(client_req);

		//execute commands
		if (cmd[0].compare("cd") == 0)
		{
			addWriter();
			f->changeDir(user, cmd[1]);
			removeWriter();
		}

		else if (cmd[0].compare("mkdir") == 0)
		{
			addWriter();
			f->createDir(user, cmd[1]);
			removeWriter();
		}

		else if (cmd[0].compare("mkfile") == 0)
		{
			f->createFile(user, cmd[1]);
		}

		else if (cmd[0].compare("delfile") == 0)
		{
			addWriter();
			f->delFile(user, cmd[1]);
			removeWriter();
		}

		else if (cmd[0].compare("rdfile") == 0)
		{
			addReader();
			if (cmd[1].compare("-o") == 0)
			{
				ss << f->readFile(user, cmd[2]);
			}
			else if (cmd[1].compare("-of") == 0)
			{
				int offset = atoi(cmd[2].c_str());
				int size = atoi(cmd[3].c_str());
				ss << f->readFile(user, cmd[2], offset, size);
			}
			else
				ss << "Invalid Mode" << endl;
			removeReader();

		}

		else if (cmd[0].compare("trfile") == 0)
		{
			int size = atoi(cmd[2].c_str());
			addWriter();
			f->truncate(user, cmd[1], size);
			removeWriter();
		}

		else if (cmd[0].compare("wrtfile") == 0)
		{
			if (cmd[1].compare("-o") == 0)
			{
				u->waiting = !u->waiting;
				if (u->waiting)
				{
					//push current parametrs into vector 
					u->cmd_data.push_back(cmd[0]);
					u->cmd_data.push_back(cmd[1]);
					u->cmd_data.push_back(cmd[2]);
					//wait for data from client
					return cmd[2] + ": ";
				}
				else
				{
					addWriter();
					f->writeToFile(user, cmd[2], cmd[3]);
					u->cmd_data.clear();
					removeWriter();
				}

			}
			else if (cmd[1].compare("-of") == 0)
			{
				u->waiting = !u->waiting;
				if (u->waiting)
				{
					//push current parametrs into vector 
					u->cmd_data.push_back(cmd[0]);
					u->cmd_data.push_back(cmd[1]);
					u->cmd_data.push_back(cmd[2]);
					u->cmd_data.push_back(cmd[3]);
					//wait for data from client
					return cmd[2] + ": ";
				}
				else
				{
					addWriter();
					f->writeToFile(user, cmd[2], cmd[4], atoi(cmd[3].c_str()));
					u->cmd_data.clear();
					removeWriter();
				}
			}
			else
				ss << "Invalid Mode" << endl;

		}

		else if (cmd[0].compare("ls") == 0)
		{
			addReader();
			ss << f->listFiles(user);
			removeReader();
		}

		else if (cmd[0].compare("map") == 0)
		{
			addReader();
			ss << f->showMemoryMap();
			removeReader();
		}

		else if (cmd[0].compare("exit") == 0)
		{
			return "exit";
		}

		else if (cmd[0].compare("help") == 0)
		{
			ss << "Commands provided are:" << endl;
			ss << "CD\tDisplays the name of or changes the current directory." << endl;
			ss << "MKDIR\tCreates a directory." << endl;
			ss << "MKFILE\tCreates a file in the directory." << endl;
			ss << "DELFILE\tDeletes the file." << endl;
			ss << "RDFILE\tReads the file" << endl;
			ss << "TRFILE\tTruncates the files" << endl;
			ss << "WRTFILE\tWrites to a file." << endl;
			ss << "LS\tList all the files" << endl;
			ss << "MAP\tShows the memory mapping." << endl;
			ss << "CLS\tClear the screen" << endl;
			ss << "EXIT\tExit the program" << endl;
		}
		
		else if (cmd[0].compare("cls") == 0)
			return "cls";

		else if (cmd[0].compare(" "))
			return("<" + this->f->path(user) + ">");

		else
			ss << "'" << cmd[0] << "'" << " is not recognized as an internal or external command, operable program or batch file." << endl;

		ss << "<" << this->f->path(user) << ">";
		return ss.str();
	}

	~DistributedFileSystem()
	{
		this->f->~FileSystem();
	}

};

class Server
{
private:
	SOCKET serverSocket;
	int port;
	DistributedFileSystem* f;
	//structre of data to pass to serveClient Func
	struct Data
	{
		DistributedFileSystem* fileSystem;
		SOCKET client;
	};

public:
	Server(int port, DistributedFileSystem* f)
	{
		this->port = port;
		this->f = f;

		WSADATA winsockData;

		int didWSAStart = WSAStartup(MAKEWORD(2, 2), &winsockData);

		if (didWSAStart != 0)
		{
			cout << "Cannot start WSA" << endl;
			return;
		}

		//create a socket
		this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		//bind the socket to ip and port
		sockaddr_in sockaddrServer;
		sockaddrServer.sin_family = AF_INET;
		sockaddrServer.sin_addr.s_addr = INADDR_ANY;
		sockaddrServer.sin_port = htons(90);

		int didBind = bind(this->serverSocket, (sockaddr*)&sockaddrServer, sizeof(sockaddrServer));

		if (didBind == SOCKET_ERROR)
		{
			cout << "Cannot Bind: " << WSAGetLastError() << endl;
			return;
		}
	}

	void start(int backlog)
	{
		//tell the winsock the socket is for listening
		int islistening = listen(this->serverSocket, backlog); //can handle max acklog clients
		if (islistening == SOCKET_ERROR)
		{
			cout << "Cannot listen: " << WSAGetLastError() << endl;
			return;
		}

		cout << "Server Listening on port: " << port << endl;
		//accept connections from client
		struct sockaddr_in clientSockData;
		int clientlen = sizeof(clientSockData);
		SOCKET clientSocket;

		while (clientSocket = accept(this->serverSocket, (sockaddr*)&clientSockData, &clientlen))
		{

			//make struct to pass data to thread
			Data* d = new Data;
			d->client = clientSocket;
			d->fileSystem = f;

			char host[NI_MAXHOST];  //Client's remote name
			char service[NI_MAXSERV];  //Servirce (i.e port) Client is connected to

			ZeroMemory(host, NI_MAXHOST);
			ZeroMemory(service, NI_MAXSERV);

			if (getnameinfo((sockaddr*)&clientSockData, sizeof(clientSockData), host, NI_MAXHOST, service, NI_MAXSERV, 0) == 0)
				cout << clientSocket << " -> " << host << " connected on: " << service << endl;

			else
			{
				inet_ntop(AF_INET, &clientSockData.sin_addr, host, NI_MAXHOST);
				cout << clientSocket << " -> " << host << " connected on: " << ntohs(clientSockData.sin_port) << endl;
			}

			//make new thread and pass client data to the thread
			_beginthreadex(0, 0, serveClient, (void*)d, 0, 0);
		}
	}

private:

	static unsigned __stdcall serveClient(void* data)
	{
		//destructring data passed
		Data* d = (Data*)data;
		SOCKET client = d->client;
		DistributedFileSystem* fileSystem = d->fileSystem;

		char recvBuffer[4096];
		string res;

		int bytes = recv(client, recvBuffer, 4096, 0);
		int user = fileSystem->addUser(recvBuffer);

		try {
			res = fileSystem->execute(user, " ");
		}
		catch (exception e)
		{
			res = e.what();
		}

		send(client, res.c_str(), res.length(), 0);

		while (true)
		{

			ZeroMemory(recvBuffer, 4096);
			int bytes = recv(client, recvBuffer, 4096, 0);
			cout << client << ": " << recvBuffer << endl;

			res = fileSystem->execute(user, recvBuffer);
			if (bytes <= 0)
			{
				//drop the client
				cout << client << " dropped";
				closesocket(client);
				break;
			}
			else
			{
				//return response from exceute function
				int didsend = send(client, res.c_str(), res.length(), 0);
			}
		}
		fileSystem->~DistributedFileSystem();
		return 0;
	}


};

int main()
{
	DistributedFileSystem* f = new DistributedFileSystem(2);
	int port = 90;

	Server* s = new Server(port, f);
	s->start(2);

	f->~DistributedFileSystem();
}

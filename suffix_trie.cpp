#include <iostream>
#include <string>
using namespace std;

class TrieNode {
public:
    char data;
    TrieNode* children[26];
    bool isTerminal;

    TrieNode(char ch) {
        data = ch;
        for (int i = 0; i < 26; i++)
            children[i] = NULL;
        isTerminal = false;
    }
};

class SuffixTrie {
public:
    TrieNode* root;

    SuffixTrie() {
        root = new TrieNode('\0');
    }

    // Convert any character to index 0–25
    int getIndex(char c) {
        c = toupper(c);
        return c - 'A';
    }

    // Insert single suffix
    void insert(TrieNode* node, const string &word, int pos) {

        // If reached end of a suffix
        if (pos == word.length()) {
            node->isTerminal = true;
            return;
        }

        int index = getIndex(word[pos]);
        if (index < 0 || index >= 26) return;  // ignore non A–Z

        if (node->children[index] == NULL)
            node->children[index] = new TrieNode(toupper(word[pos]));

        insert(node->children[index], word, pos + 1);
    }

    // Insert all suffixes
    void insertWord(const string &word) {
        for (int i = 0; i < word.length(); i++)
            insert(root, word, i);
    }

    // Search a substring
    bool search(TrieNode* node, const string &word, int pos) {

        if (pos == word.length())
            return true;

        int index = getIndex(word[pos]);
        if (index < 0 || index >= 26)
            return false;

        if (node->children[index] == NULL)
            return false;

        return search(node->children[index], word, pos + 1);
    }

    bool searchWord(const string &word) {
        return search(root, word, 0);
    }
};

int main() {
    SuffixTrie t;
    t.insertWord("BANANA");

    cout << (t.searchWord("ANA") ? "Present" : "Not Present") << endl;
    cout << (t.searchWord("NANA") ? "Present" : "Not Present") << endl;
    cout << (t.searchWord("BAN") ? "Present" : "Not Present") << endl;
    cout << (t.searchWord("APPLE") ? "Present" : "Not Present") << endl;

    return 0;
}


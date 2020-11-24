#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <cmath>

using namespace std;

void fill_blocks(vector<string>& vec, const string& text, unsigned int size) {
    unsigned int dot = 0;
    unsigned int add_space = size - text.length()%size;
    char random_char = static_cast<char>(rand()%128);
    string new_text = text;
    for (int i = 0; i < add_space; ++i) {
        new_text += random_char;
    }
    for (int i = 0; i < new_text.length()/size; ++i) {
        vec.push_back(new_text.substr(dot, size));
        dot += size;
    }
}

void create_gamma(unsigned int size, long long& gamma, string& str_gamma) {
    gamma = 0;
    int random;
    for (int i = 0; i < size / 2; ++i) {
        random = rand() % 94 + 33;
        gamma += gamma * 256 + random;
        str_gamma += static_cast<char>(random);
    }
}

string make_binary(int digit) {
    string result;
    while (digit > 0) {
        result += to_string(digit % 2);
        digit /= 2;
    }
    reverse(result.begin(), result.end());
    while (result.length()%8 != 0) {
        result = "0" + result;
    }
    return result;
}

string make_byte_view(const string& str) {
    string result;
    for (const char& sym : str) {
        int num_sym = static_cast<unsigned char>(sym);
        string binary = make_binary(num_sym);
        result += binary;
    }
    return result;
}

long long make_decimal(string& str) {

    long long result = 0;
    int count = 0;
    while (str[0] == '0') {
        str.erase(0, 1);
    }
    while (!str.empty()) {
        result += static_cast<long long>((str[str.length() - 1] - '0') * pow(2, count));
        str.erase(str.length() - 1);
        ++count;
    }
    return result;
}

void do_shift(string& str, unsigned int shift, const string& where) {
    string result;
    if (where == "<<") {
        rotate(str.begin(), str.begin() + shift, str.end());
    } else {
        rotate(str.rbegin(), str.rbegin() + shift, str.rend());
    }
    while (!str.empty()) {
        string add = str.substr(0, 8);
        result += static_cast<char>(make_decimal(add));
        str.erase(0, 8);
    }
    str = result;
}

int main() {
    ifstream input("input.txt");
    if (!input) {
        cerr << "File is not open!\n";
        return -1;
    }

    srand(15);

    unsigned int shift;
    cout << "Enter shift: ";
    cin >> shift;

    string plaintext;
    getline(input, plaintext); // получаем plaintext
    input.close();

    unsigned int size_of_block;
    cout << "Enter size of block: ";
    cin >> size_of_block;

    vector<string> vec_of_blocks;
    fill_blocks(vec_of_blocks, plaintext, size_of_block);

    long long gamma[2];

    string gamma_str[2];

    for (string& i : vec_of_blocks) {
        create_gamma(size_of_block, gamma[0], gamma_str[0]); // создаем две части гаммы
        create_gamma(size_of_block, gamma[1], gamma_str[1]);
        string full_gamma = gamma_str[0] + gamma_str[1]; //полная гамма
        for (int j = 0; j < i.length(); ++j) {
            i[j] ^= full_gamma[j]; //каждый байт гаммы побитово ксорим на соответсвенный байт шифротекста
        }
    }


    for (string& i : vec_of_blocks) {
        string byte_view = make_byte_view(i); //представляем в байтовом виде
        do_shift(byte_view, shift, "<<"); //выполняем сдвиг на shift
        i = byte_view;
    }

    ofstream encrypt("Encrypted.txt");

    encrypt << "Encrypted: " << endl;

    for (const string& i : vec_of_blocks) {
        encrypt << i;  //выводим в файл шифротекст
    }

    encrypt.close();

    //Decrypting...

    ofstream decrypt("Decrypted.txt");

    for (string& i : vec_of_blocks) {
        string byte_view = make_byte_view(i);  //представляем в байтовом виде
        do_shift(byte_view, shift, ">>"); //сдвигаем в обратную строну на shift
        i = byte_view;
    }

    srand(15);

    for (string& i : vec_of_blocks) {
        create_gamma(size_of_block, gamma[0], gamma_str[0]); //создаем гамму с тем же srand
        create_gamma(size_of_block, gamma[1], gamma_str[1]);
        string full_gamma = gamma_str[0]+gamma_str[1];
        for (int j = 0; j < i.length(); ++j) {
            i[j] ^= full_gamma[j]; //ксорим...
        }
    }

    decrypt << "Decrypted:" << endl;

    for (const string& i : vec_of_blocks) {
        decrypt << i;  //выводим в файл первоначальный plaintext
    }

    return 0;
}

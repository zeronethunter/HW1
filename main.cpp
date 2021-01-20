//элементы командной строки вида:
//input.txt encrypt.txt decrypt.txt 4(сдвиг) 16(размер блока)

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <ctime>

using namespace std;

struct Block { //стркутура блок
    string block; //строковый блок
    unsigned char g_block[16]; //значения каждого байта блока, используем 1байтовый беззнаковый тип
    void clear() { //очистка
        block.clear();
        for (auto& i : g_block) {
            i = 0;
        }
    }
};

Block operator^(const Block& first, const Block& second) {
    Block new_block;
    int size = first.block.size();
    for (int i = 0; i < size; ++i) {
        new_block.block.push_back(static_cast<unsigned char>(first.block[i]) ^ static_cast<unsigned char>(second.block[i]));  //ксорим каждый соответствующий символ
        new_block.g_block[i] = new_block.block[i]; //записываем новое значение
    }
    return new_block;
}

Block operator>> (Block block, unsigned int size) {
    Block new_block;
    unsigned char lost_bits[16];
    for (int i = 0; i < 16; ++i) {
        lost_bits[i] = block.g_block[i] << (8 - size); //записываем то, что мы теряем
        new_block.g_block[i] = block.g_block[i] >> size; //делаем сдвиг
    }
    for (int i = 1; i < 16; ++i) {
        new_block.g_block[i] |= lost_bits[i - 1]; //то, что мы теряем переходит в другой байт правее
    }
    new_block.g_block[0] |= lost_bits[15]; //потери последнего байта идут в первый, так так кольцевой сдвиг
    for (unsigned char i : new_block.g_block) {
        new_block.block.push_back(static_cast<unsigned char>(i));
    }
    return new_block;
}

Block operator<< (Block block, unsigned int size) { //аналогично сдвигу вправо
    Block new_block;
    unsigned char lost_bits[16];
    for (int i = 0; i < 16; ++i) {
        lost_bits[i] = block.g_block[i] >> (8 - size);
        new_block.g_block[i] = block.g_block[i] << size;
    }
    for (int i = 0; i < 15; ++i) {
        new_block.g_block[i] |= lost_bits[i + 1];
    }
    new_block.g_block[15] |= lost_bits[0];
    for (unsigned char i : new_block.g_block) {
        new_block.block.push_back(static_cast<unsigned char>(i));
    }
    return new_block;
}

void fill_blocks(vector<Block>& vec, unsigned int size) { //заполнение заглушкой
    unsigned int add_space = (size - vec[vec.size() - 1].block.length() % size) % size;
    auto random_char = static_cast<unsigned char>(rand()%128);
    for (int i = 0; i < add_space; ++i) {
        vec[vec.size() - 1].block += random_char;
    }
}

void create_gamma(unsigned int size, Block& gamma) { //создаем гамму
    unsigned char random;
    string str_gamma;
    for (int i = 0; i < size; ++i) {
        random = rand() % 94 + 33;  //псевдорандомные "хорошие" символы
        gamma.g_block[i] = random;
        str_gamma += random;
    }
    gamma.block = str_gamma;
}

int main(int argc, char *argv[]) {
    if (argc > 6) {
        cout << "Set correct arguments of command line!";
        return -1;
    }
    string input_file = argv[1];
    string encrypt_file = argv[2];
    string decrypt_file = argv[3];
    unsigned int shift = stoi(argv[4]);
    unsigned int size_of_block = stoi(argv[5]);
    vector<Block> vec_of_blocks;

    ifstream input(input_file, ios::binary);
    if (!input) {
        cerr << "File is not open!\n";
        return -1;
    }

    srand(time(nullptr));

    const int VI = rand();  //получаем вектор инициализации

    string plaintext;
    plaintext.resize(size_of_block);
    while (!input.eof()) {
        input.read(plaintext.data(), size_of_block);         // считываю блоки
        plaintext = plaintext.substr(0, input.gcount());
        for (char& i : plaintext) {
            i = static_cast<unsigned char>(i);
        }
        Block new_block = {plaintext};
        vec_of_blocks.push_back(new_block);
    }
    input.close();

    int count_blocks = vec_of_blocks.size();  //запоминаю, чтобы в последующем исбавиться от мусора

    srand(VI); // секретное число

    fill_blocks(vec_of_blocks, size_of_block); // используем заглушку

    for (Block& i : vec_of_blocks) {
        for (int j = 0; j < size_of_block; ++j) {
            i.g_block[j] = static_cast<unsigned char>(i.block[j]);  //пихаем байты в массив с байтами
        }
    }

    Block MyGamma;

    for (Block& i : vec_of_blocks) {
        create_gamma(size_of_block, MyGamma); // создаем две части гаммы
        i = i ^ MyGamma;  //ксорим
    }

    for (Block& i : vec_of_blocks) {
        i = i << shift;  //сдвигаем
    }

    ofstream encrypt(encrypt_file);

    for (const Block i : vec_of_blocks) {
        encrypt << i.block; //выводим в файл шифротекст
    }
    encrypt.close();


//Decrypting...

    ofstream decrypt(decrypt_file);

    ifstream encrypt_in(encrypt_file, ios::binary);

    srand(VI);

    auto decrypt_char = static_cast<unsigned char>(rand()%128);  //получаю старую заглушку

    vector<Block> de_blocks;

    plaintext.resize(size_of_block);

    while (!encrypt_in.eof()) {
        encrypt_in.read(plaintext.data(), size_of_block);         // считываю блоки из шифротекста
        for (char& i : plaintext) {
            i = static_cast<unsigned char>(i);
        }
        Block new_block = {plaintext};
        de_blocks.push_back(new_block);
    }
    encrypt_in.close();

    for (Block& i : de_blocks) {
        for (int j = 0; j < size_of_block; ++j) {
            i.g_block[j] = static_cast<unsigned char>(i.block[j]); //пихаю байты
        }
    }

    for (Block& i : de_blocks) {
        i = i >> shift; //делаю обратный сдвиг
    }

    MyGamma.clear();

    for (Block& i : de_blocks) {
        create_gamma(size_of_block, MyGamma); // создаем две части гаммы
        i = i ^ MyGamma;  //ксорим
    }

    for (int i = 0; i < size_of_block; ++i) { //избавляюсь от заглушки
        if (de_blocks[count_blocks - 1].g_block[i] == decrypt_char) {
            de_blocks[count_blocks - 1].block.erase(i, size_of_block - i + 1);
            break;
        }
    }

    decrypt << "Decrypted:" << endl;

    for (int i = 0; i < count_blocks; ++i) { //вывожу без мусора
        decrypt << de_blocks[i].block;
    }

    decrypt.close();
    return 0;
}

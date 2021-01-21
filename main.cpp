#include <iostream>
#include <string>
#include <fstream>
#include <vector>

using namespace std;

//Ввод вида
//C:\Users\...\source\repos\...\...\input.txt C:\Users\...\source\repos\...\...\encrypt.txt 4 16 228 encrypt
//C:\Users\...\source\repos\...\...\encrypt.txt C:\Users\...\source\repos\...\...\decrypt.txt 4 16 228 decrypt

struct Block { //стркутура блок
    unsigned char g_block[16]; //значения каждого байта блока, используем 1байтовый беззнаковый тип
    Block() {
        for (unsigned char &i : g_block) {
            i = 0;
        }
    }
    int true_size() const {
        int count = 0;
        for (auto i : g_block) {
            if (i != 0) {
                ++count;
            }
        }
        return count;
    }
    string make_str() const {
        string result;
        for (auto i : g_block) {
            result.push_back(i);
        }
        return result;
    }
    int size() const {
        return sizeof(g_block);
    }
};

Block operator^(const Block& first, const Block& second) {
    Block new_block;
    for (int i = 0; i < new_block.size(); ++i) {
        new_block.g_block[i] = first.g_block[i] ^ second.g_block[i]; //записываем новое значение
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
    return new_block;
}

void fill_blocks(vector<Block>& vec) { //заполнение заглушкой
    auto random_char = static_cast<unsigned char>(rand()%128);
    for (int i = vec[vec.size() - 1].true_size(); i < vec[vec.size() - 1].size(); ++i) {
        vec[vec.size() - 1].g_block[i] = random_char;
    }
}

void create_gamma(Block& gamma) { //создаем гамму
    for (auto& i : gamma.g_block) {
        i = rand() % 94 + 33;  //псевдорандомные "хорошие" символы
    }
}

int main(int argc, char *argv[]) {
    if (argc > 7) {
        cout << "Set correct arguments of command line!";
        return -1;
    }
    string input_file = argv[1];
    string output_file = argv[2];
    unsigned int shift = stoi(argv[3]);
    unsigned int size_of_block = stoi(argv[4]);
    int VI = stoi(argv[5]);
    string mode = argv[6];
    vector<Block> vec_of_blocks;

    ifstream input(input_file, ios::binary);
    ofstream output(output_file);
    if (!input) {
        cerr << "File is not open!\n";
        return -1;
    }

    string plaintext;
    plaintext.resize(size_of_block);
    int count_blocks = 0;  //запоминаю, чтобы в последующем избавиться от мусора
    while (!input.eof()) {
        input.read(plaintext.data(), size_of_block);         // считываю блоки
        Block new_block;
        count_blocks += input.gcount();
        for (int i = 0; i < input.gcount(); ++i) {
            new_block.g_block[i] = plaintext[i];
        }
        vec_of_blocks.push_back(new_block);
    }
    input.close();
    count_blocks /= size_of_block;

    if (mode == "encrypt") {

        srand(VI); // секретное число

        fill_blocks(vec_of_blocks); // используем заглушку

        Block MyGamma;

        for (Block &i : vec_of_blocks) {
            create_gamma(MyGamma); // создаем две части гаммы
            i = i ^ MyGamma;  //ксорим
        }

        for (Block &i : vec_of_blocks) {
            i = i << shift;  //сдвигаем
        }

        for (const Block& block : vec_of_blocks) {
            output << block.make_str(); //выводим в файл шифротекст
        }
        output.close();

    } else if (mode == "decrypt"){

        //Decrypting...

        srand(VI); // секретное число

        auto decrypt_char = static_cast<unsigned char>(rand() % 128);  //получаю старую заглушку

        for (Block &i : vec_of_blocks) {
            i = i >> shift; //делаю обратный сдвиг
        }

        Block MyGamma;

        for (Block &i : vec_of_blocks) {
            create_gamma(MyGamma); // создаем две части гаммы
            i = i ^ MyGamma;  //ксорим
        }

        Block tmp = vec_of_blocks[count_blocks - 1];

        string str_tmp = tmp.make_str();

        for (int i = 0; i < size_of_block; ++i) { //избавляюсь от заглушки
            if (str_tmp[i] == decrypt_char) {
                str_tmp.erase(i, str_tmp.size() - i + 1);
                break;
            }
        }

        output << "Decrypted:" << endl;

        for (int i = 0; i < count_blocks - 1; ++i) { //вывожу без мусора
            output << vec_of_blocks[i].make_str();
        }
        output << str_tmp;

        output.close();
    }
    return 0;
}

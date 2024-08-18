#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

const double INITIAL_PROFIT = 0;
const double INITIAL_MONEY = 0;
const int DEGHAT = 2;
const int PERCENT = 100;
const int INVALID_RETURN = 0;
const int NOT_MONEY_RETURN = 1;
const string INVALID_SHORT_TERM = "Invalid short-term deposit";
const string OK = "OK";
const string NOT_ENOUGH_MONEY = "Not enough money";
const string DONE = "done";
const string USER_INST = "-u";
const string BANK_INST = "-b";
const char SEPERATOR = ',';
const char EOCSV = '\r';
const string SHORT_TERM_DEPOSTIT_COMMAND = "create_short_term_deposit";
const string LONG_TERM_DEPOSTIT_COMMAND = "create_long_term_deposit";
const string GHARZ_DEPOSTIT_COMMAND = "create_gharzolhasane_deposit";
const string PASS_TIME_COMMAND = "pass_time";
const string INVENTORY_REPORT_COMMAND = "inventory_report";
const string CALC_MONEY_IN_BANK_COMMAND = "calc_money_in_bank";
const string CALC_ALL_MONEY_COMMAND = "calc_all_money";


struct ShortTermDeposit
{
    int id = 0;
    int userId;
    int bankId;
    double initialInvest;
    double profit = 0;
};

struct LongTermDeposit
{
    int userId;
    int bankId;
    int shortTermId;
    int years;
    double initialInvest;
    double profit = 0;
};

struct GharzDeposit
{
    int userId;
    int bankId;
    double initialInvest;
};

class User {
private:
    int id;
    double wallet;
public:
    User(int _id, double _wallet) {
        id = _id;
        wallet = _wallet;
    }
    int getId() {return id;}
    double getWallet() {return wallet;}
    void deductWallet(double amount) {wallet -= amount;}
};

class Bank {
private:
    int id;
    double shortTermProfitMargin;
    double shortTermMinimumInvestment;
    vector<ShortTermDeposit> shortTermDiposit;
    vector<LongTermDeposit> longTermDeposit;
    vector<GharzDeposit> gharzDeposit;

public:
    Bank(int _id, double _ShortTermProfitMargin, double _shortTermMinimumInvestment) {
        id = _id;
        shortTermProfitMargin = _ShortTermProfitMargin;
        shortTermMinimumInvestment = _shortTermMinimumInvestment;
    }

    int getId() {return id;}
    double getShortTermProfitMargin() {return shortTermProfitMargin;}
    double getShortTermMinimumInvestment() {return shortTermMinimumInvestment;}
    void addShortTermDeposit(ShortTermDeposit& deposit) {shortTermDiposit.push_back(deposit);}
    void addLongTermDeposit(LongTermDeposit& deposit) {longTermDeposit.push_back(deposit);}
    void addGharzDeposit(GharzDeposit& deposit) {gharzDeposit.push_back(deposit);}
    void genShortTermId(ShortTermDeposit& deposit);
    int checkShortTermCondition(User* user, double initialInvestment);
    int checkLongTermCondition(User* user, double initialInvestment, int years, int shortTermId);
    int checkGharzCondition(User* user, double initialInvestment);
    int checkShortTermAvailable(int userId, int shortTermId);
    void passTime(int month);
    ShortTermDeposit* findShortDepositById(int id);
    double calcShortTermProfits(int UserId);
    double calcLongTermProfits(int UserId);
    double calcGharzProfits(int UserId);
};

class BankSystem {
private:
    vector<Bank> banks;
    vector<User> users;
public:
    void loadFiles(string banksFileDir, string usersFileDir);
    void readUsersCsv(string fileName);
    void readBanksCsv(string fileName);
    void getCommand();
    void createShortTermDep(int userId, int bankId, double initialInvestment);
    void createLongTermDep(int userId, int bankId, int shortTermDepositId, int years, double initialInvestment);
    void createGharzolDep(int userId, int bankId, double initialInvestment);
    Bank* findBankById(int id);
    User* findUserById(int id);
    void passTime(int month);
    void inventoryReport(int userId, int bankId, int shortTermDepId);
    void calcMoneyInBank(int userId, int bankId);
    void calcAllMoney(int userId);
};



double Bank::calcShortTermProfits(int UserId){
    double profit = INITIAL_PROFIT;
    for(ShortTermDeposit tmp : shortTermDiposit){
        if(tmp.userId == UserId)
            profit += (tmp.profit + tmp.initialInvest);
    }
    return profit;
}

double Bank::calcLongTermProfits(int UserId){
    double profit = INITIAL_PROFIT;
    for(LongTermDeposit tmp : longTermDeposit){
        if(tmp.userId == UserId)
            profit += (tmp.initialInvest);
    }
    return profit;
}

double Bank::calcGharzProfits(int UserId){
    double profit = INITIAL_PROFIT;
    for(GharzDeposit tmp : gharzDeposit){
        if(tmp.userId == UserId)
            profit += (tmp.initialInvest);
    }
    return profit;
}

void BankSystem::calcAllMoney(int userId){
    double money = INITIAL_MONEY;
    for(Bank tmpBank: banks){
        double shortTermProfits = tmpBank.calcShortTermProfits(userId);
        double longTermProfits = tmpBank.calcLongTermProfits(userId);
        double GharzProfits = tmpBank.calcGharzProfits(userId);
        money += shortTermProfits + longTermProfits + GharzProfits;
    }
    cout << fixed << setprecision(2) << money << endl;
}

void BankSystem::calcMoneyInBank(int userId, int bankId){
    Bank* foundedBank = findBankById(bankId);
    double shortTermProfits = foundedBank->calcShortTermProfits(userId);
    double longTermProfits = foundedBank->calcLongTermProfits(userId);
    double GharzProfits = foundedBank->calcGharzProfits(userId);
    double money = shortTermProfits + longTermProfits + GharzProfits;
    cout << fixed << setprecision(DEGHAT) << money << endl;
}

void BankSystem::inventoryReport(int userId, int bankId, int shortTermDepId){
    Bank* foundedBank = findBankById(bankId);
    User* foundedUser = findUserById(userId);
    ShortTermDeposit* foundedDeposit = foundedBank->findShortDepositById(shortTermDepId);
    if(foundedBank->checkShortTermAvailable(userId, shortTermDepId))
        cout << fixed << setprecision(2) << foundedDeposit->profit + foundedDeposit->initialInvest << endl;
    else cout << INVALID_SHORT_TERM << endl;
}

ShortTermDeposit* Bank::findShortDepositById(int id){
    for(ShortTermDeposit& tmp : shortTermDiposit){
        if(tmp.id == id){
            return &tmp;
        }
    }
    return nullptr;
}

void Bank::passTime(int month){
    for(ShortTermDeposit& tmp: shortTermDiposit){
        double darsad = getShortTermProfitMargin() * month / PERCENT;
        tmp.profit += darsad * tmp.initialInvest;
    }
    for(LongTermDeposit& tmp: longTermDeposit){
        double darsad = getShortTermProfitMargin() * tmp.years * month / PERCENT;
        tmp.profit = darsad * tmp.initialInvest;
        ShortTermDeposit* foundedDeposit = findShortDepositById(tmp.shortTermId);
        foundedDeposit->profit += tmp.profit;
    }
}

void BankSystem::passTime(int month){
    for (int i = 0; i < banks.size(); i++)
        banks[i].passTime(month);
    cout << OK << endl;
}

int Bank::checkGharzCondition(User* user, double initialInvestment){
    if(user->getWallet() < initialInvestment) 
        return INVALID_RETURN;
    return 1;
}

int Bank::checkShortTermCondition(User* user, double initialInvestment){
    if(user->getWallet() < initialInvestment || initialInvestment < shortTermMinimumInvestment) 
        return INVALID_RETURN;
    return 1;
}

int Bank::checkLongTermCondition(User *user, double initialInvestment, int years, int shortTermId){
    if (user == nullptr || checkShortTermAvailable(user->getId(), shortTermId) == 0){
        return INVALID_RETURN;
    } 
    else if (initialInvestment > user->getWallet() ||initialInvestment < shortTermMinimumInvestment*years )
        return NOT_MONEY_RETURN;
    return 2;
}

int Bank::checkShortTermAvailable(int userId, int shortTermId){
    int isFound = 0;
    for (ShortTermDeposit tmp : shortTermDiposit) {
        if (tmp.userId == userId && tmp.id == shortTermId){ 
            isFound = 1;
            return isFound;
        }
    }
    return isFound;
}

void Bank::genShortTermId(ShortTermDeposit& deposit){
    int numOfId = 0;
    for (ShortTermDeposit& tmp : shortTermDiposit) {
        if (deposit.bankId == tmp.bankId) 
            numOfId++;
    }
    deposit.id += numOfId + 1;
}

void BankSystem::createGharzolDep(int userId, int bankId, double initialInvestment){
    Bank* foundedBank = findBankById(bankId);
    User* foundedUser = findUserById(userId);
    GharzDeposit tmp;
    if(1/* foundedBank->checkGharzCondition(foundedUser, initialInvestment) */){
        tmp.userId = userId;
        tmp.bankId = bankId;
        tmp.initialInvest = initialInvestment;
        //foundedUser->deductWallet(initialInvestment);
        foundedBank->addGharzDeposit(tmp);
        cout << OK << endl;
    }
    else cout << NOT_ENOUGH_MONEY << endl;
}

void BankSystem::createLongTermDep(int userId, int bankId, int shortTermDepositId, int years, double initialInvestment){
    Bank* foundedBank = findBankById(bankId);
    User* foundedUser = findUserById(userId);
    LongTermDeposit tmp;
    if(foundedBank->checkLongTermCondition(foundedUser, initialInvestment, years, shortTermDepositId) == 2){
        tmp.userId = userId;
        tmp.bankId = bankId;
        tmp.shortTermId = shortTermDepositId;
        tmp.years = years;
        tmp.initialInvest = initialInvestment;
        foundedUser->deductWallet(initialInvestment);
        foundedBank->addLongTermDeposit(tmp);
        cout << OK << endl;
    }
    else if(foundedBank->checkLongTermCondition(foundedUser, initialInvestment, years, shortTermDepositId) == NOT_MONEY_RETURN) {
        cout << NOT_ENOUGH_MONEY << endl;
    }
    else cout << INVALID_SHORT_TERM << endl;
}

void BankSystem::createShortTermDep(int userId, int bankId, double initialInvestment){
    Bank* foundedBank = findBankById(bankId);
    User* foundedUser = findUserById(userId);
    ShortTermDeposit tmp;
    if(foundedBank->checkShortTermCondition(foundedUser, initialInvestment)){
        tmp.userId = userId;
        tmp.bankId = bankId;
        tmp.initialInvest = initialInvestment;
        foundedBank->genShortTermId(tmp);
        foundedUser->deductWallet(initialInvestment);
        foundedBank->addShortTermDeposit(tmp);
        cout << tmp.id << endl;
    }
    else{
        cout << NOT_ENOUGH_MONEY << endl;
    }
}

Bank* BankSystem::findBankById(int id){
    for(Bank& tmp : banks){
        if(tmp.getId() == id){
            return &tmp;
        }
    }
    return nullptr;
}

User* BankSystem::findUserById(int id){
    for(User& tmp : users){
        if(tmp.getId() == id){
            return &tmp;
        }
    }
    return nullptr;
}

vector<string> split_text(string text, char seperator){
    vector<string> result;
    string token;
    int start = 0;
    int end = text.find(seperator);
    while (end != -1) {
        token = text.substr(start, end - start);
        result.push_back(token);
        start = end + 1;
        end = text.find(seperator, start);
    }
    token = text.substr(start, end);
    result.push_back(token);
    return result;
}

void BankSystem::getCommand(){
    string request;
    while(cin >> request){
        if(request == SHORT_TERM_DEPOSTIT_COMMAND){
            int userId, bankId;
            double initialInvestment;
            cin >> userId >> bankId >> initialInvestment;
            createShortTermDep(userId, bankId, initialInvestment);
        }
        else if(request == LONG_TERM_DEPOSTIT_COMMAND){
            int userId, bankId, shortTermDepositId, years;
            double initialInvestment;
            cin >> userId >> bankId >> shortTermDepositId >> years >> initialInvestment;
            createLongTermDep(userId, bankId, shortTermDepositId, years, initialInvestment);
        }
        else if(request == GHARZ_DEPOSTIT_COMMAND){
            int userId, bankId;
            double initialInvestment;
            cin >> userId >> bankId >> initialInvestment;
            createGharzolDep(userId, bankId, initialInvestment);
        }
        else if(request == PASS_TIME_COMMAND){
            int month;
            cin >> month;
            passTime(month);
        }
        else if(request == INVENTORY_REPORT_COMMAND){
            int userId, bankId, shortTermDepositId;
            cin >> userId >> bankId >> shortTermDepositId;
            inventoryReport(userId, bankId, shortTermDepositId);
        }
        else if(request == CALC_MONEY_IN_BANK_COMMAND){
            int userId, bankId;
            cin >> userId >> bankId;
            calcMoneyInBank(userId, bankId);
        }
        else if(request == CALC_ALL_MONEY_COMMAND){
            int userId;
            cin >> userId;
            calcAllMoney(userId);
        }
    }
}

void BankSystem::readUsersCsv(string fileName){
    ifstream userFile(fileName);
    string line;
    getline(userFile, line);
    while (getline(userFile, line))
    {
        if (!line.empty() && line.back() == EOCSV) {
            line.pop_back();
        }
        vector<string> data = split_text(line, SEPERATOR);
        User tmpUser(stoi(data[0]),stof(data[1]));
        users.push_back(tmpUser);
    }
    userFile.close();
}

void BankSystem::readBanksCsv(string fileName){
    ifstream bankFile(fileName);
    string line;
    getline(bankFile, line);
    while (getline(bankFile, line))
    {
        if (!line.empty() && line.back() == EOCSV) {
            line.pop_back();
        }
        vector<string> data = split_text(line, SEPERATOR);
        Bank tmpBank(stoi(data[0]), stoi(data[1]), stof(data[2]));
        banks.push_back(tmpBank);
    }
    bankFile.close();
}

void BankSystem::loadFiles(string banksFileDir, string usersFileDir){
    readUsersCsv(usersFileDir);
    readBanksCsv(banksFileDir);
}

void ldFilesAddr(int argc, char* argv[], string& banksFileDir, string& usersFileDir){
    banksFileDir = "banks.csv";
    usersFileDir = "users.csv";
    for (int i = 1; i < argc; i += 2) {
        string option = argv[i];
        string value = argv[i + 1];
        if (option == BANK_INST) {
            banksFileDir = value;
        } else if (option == USER_INST) {
            usersFileDir = value;
        }
    }
}

int main(int argc, char* argv[]){
    string banksFileDir, usersFileDir;
    ldFilesAddr(argc, argv, banksFileDir, usersFileDir);
    BankSystem bankSystem;
    bankSystem.loadFiles(banksFileDir, usersFileDir);
    bankSystem.getCommand();
    return 0;
}



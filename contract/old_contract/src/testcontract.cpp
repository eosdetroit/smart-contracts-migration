#include "../include/testcontract.hpp"

void testcontract::init(string contract_name, name initial_admin) {
    //authenticate
    require_auth(get_self());

    //open config singleton
    config_singleton configs(get_self(), get_self().value);

    //validate
    check(!configs.exists(), "contract already initialized");
    check(is_account(initial_admin), "initial admin account doesn't exist");

    //initialize
    config initial_conf;
    initial_conf.contract_name = contract_name;
    initial_conf.contract_version = "0.1.0";
    initial_conf.admin_acct = initial_admin;

    //set initial config
    configs.set(initial_conf, get_self());
}

void testcontract::setversion(string new_version) {
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin_acct);

    //change contract version
    conf.contract_version = new_version;

    //set new config
    configs.set(conf, get_self());
}

void testcontract::setadmin(name new_admin) {
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin_acct);

    //validate
    check(is_account(new_admin), "new admin account doesn't exist");

    //change admin
    conf.admin_acct = new_admin;

    //set new config
    configs.set(conf, get_self());
}

void testcontract::deposit(name from, name to, eosio::asset quantity, std::string memo) {
    if (from != get_self() && to == get_self()) { 
        check(quantity.amount > 0, "Negative quantity");
        check(quantity.symbol == WAX_SYM, "You are trying to deposit the wrong token");

        token_table _token(get_self(), get_self().value);
        auto token_itr = _token.find(from.value);

        if(token_itr == _token.end()) {
            _token.emplace(get_self(), [&](auto &r) {
                r.waxbalance = quantity;
                r.account = from;
            });
        } else {
            _token.modify(token_itr, get_self(), [&](auto &r) {
                r.waxbalance += quantity;
            });
        }
    }
}


void testcontract::withdraw(const name &user) {
    require_auth(user);

    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    token_table _token(get_self(), get_self().value);
    auto token_itr = _token.find(user.value);

    check (token_itr != _token.end(), "You have no token deposited!");
    
    //send inline transfer to return asset
    action(permission_level{get_self(), name("active")}, name("eosio.token"), name("transfer"), make_tuple(
        get_self(), //from
        user, //to
        token_itr->waxbalance, //amount
        "Withdrawing your money"
    )).send(); 

    _token.erase(token_itr);
}



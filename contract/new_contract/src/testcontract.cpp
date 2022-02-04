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

void testcontract::setlockdtime(uint32_t locked_days) {
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin_acct);

    //set locked days
    conf.locked_days = locked_days;

    //set new config
    configs.set(conf, get_self());
}


void testcontract::addsymbol(symbol_code token_symbol, uint32_t decimals) 
{
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin_acct);

    //Check if template_id is in the allowed list
	auto symbol_itr = std::find(conf.accepted_tokens.begin(), conf.accepted_tokens.end(), token_symbol);
	check(symbol_itr == conf.accepted_tokens.end(), "symbol token already added");

	//add new category to categories list
	conf.accepted_tokens.push_back(token_symbol);

	//set new config
    configs.set(conf, get_self());

}

void testcontract::rmvsymbol(symbol_code token_symbol, uint32_t decimals)
{
    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(conf.admin_acct);

    //validate
	auto symbol_itr = std::find(conf.accepted_tokens.begin(), conf.accepted_tokens.end(), token_symbol);
	check(symbol_itr != conf.accepted_tokens.end(), "symbol token not found");

    //Remove token_symbol
    conf.accepted_tokens.erase(symbol_itr);

    //set new config
    configs.set(conf, get_self());
}

void testcontract::deposit(name from, name to, eosio::asset quantity, std::string memo) {
    if (from != get_self() && to == get_self()) { 
        check(quantity.amount > 0, "Negative quantity");
        
        users_table _users(get_self(), get_self().value);
        auto user_itr = _users.find(from.value);
        if(user_itr == _users.end()) {
            _users.emplace(get_self(), [&](auto &r) {
                r.user = from;
            });
        }

        token_table _token(get_self(), from.value);
        auto symbol_token_idx = _token.get_index<name("symbol")>();
        auto token_idx = symbol_token_idx.find(quantity.symbol.code().raw());
        auto token_itr = _token.find(token_idx->token_id);


        if(token_itr == _token.end()) {
            _token.emplace(get_self(), [&](auto &r) {
                r.token_id = _token.available_primary_key();
                r.balance = quantity;
                r.token = quantity.symbol.code();
                r.deposit_time =  time_point_sec(current_time_point());
                r.is_locked = true;
            });
        } else {
            _token.modify(token_itr, get_self(), [&](auto &r) {
                r.balance += quantity;
                r.deposit_time = time_point_sec(current_time_point());
            });
        }

        stats_table _stats(get_self(), get_self().value);
        auto symbol_stats_idx = _stats.get_index<name("symbol")>();
        auto stats_idx = symbol_stats_idx.find(quantity.symbol.code().raw());
        auto stats_itr = _stats.find(stats_idx->stats_id);

        if(stats_itr == _stats.end()) {
            _stats.emplace(get_self(), [&](auto &r) {
                r.stats_id = _stats.available_primary_key();
                r.token_symbol = quantity.symbol.code();
                r.precision = quantity.symbol.precision();
                r.total_amount = quantity.amount;
            });
        } else {
            _stats.modify(stats_itr, get_self(), [&](auto &r){
                r.total_amount += quantity.amount;
            });
        }

    }
}


void testcontract::withdraw(const name &user, eosio::asset asset) {
    require_auth(user);

    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    token_table _token(get_self(), user.value);
    auto symbol_token_idx = _token.get_index<name("symbol")>();
    auto token_idx = symbol_token_idx.find(asset.symbol.code().raw());
    auto token_itr = _token.find(token_idx->token_id);

    check (token_itr != _token.end(), "You have no token deposited!");
    check (token_itr->balance.amount >= asset.amount, "You cannot retire more token than the staked");
    check (!token_itr->is_locked || 
            token_itr->deposit_time + conf.locked_days*86400 >= time_point_sec(current_time_point()),
            "You cannot unlock your token yet.");
    
    //send inline transfer to return asset
    action(permission_level{get_self(), name("active")}, name("eosio.token"), name("transfer"), make_tuple(
        get_self(), //from
        user, //to
        token_itr->balance, //amount
        "Withdrawing your money"
    )).send(); 

    stats_table _stats(get_self(), get_self().value);
    auto symbol_stats_idx = _stats.get_index<name("symbol")>();
    auto stats_idx = symbol_stats_idx.find(asset.symbol.code().raw());
    auto stats_itr = _stats.find(stats_idx->stats_id);

    _stats.modify(stats_itr, get_self(), [&](auto &r){
        r.total_amount -= token_itr->balance.amount;
    });

    _token.erase(token_itr);
}

// Import necessary library
#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/print.hpp>
#include <eosio/singleton.hpp>


using namespace std;
using namespace eosio;

CONTRACT testcontract : public contract {
  public: 
    using contract::contract;

    static constexpr symbol WAX_SYM = symbol("WAX", 8);

    [[eosio::on_notify("eosio.token::transfer")]]
    void deposit(name from, name to, eosio::asset quantity, std::string memo);

    ACTION withdraw(const name &user);

    ACTION init(string contract_name, name initial_admin);
    ACTION setadmin(name new_admin);
    ACTION setversion(string new_version);

  private: 

    TABLE config {
      string contract_name;
      string contract_version;
      name admin_acct;
   
      EOSLIB_SERIALIZE(config, (contract_name)(contract_version)(admin_acct))
    };
    typedef singleton<name("config"), config> config_singleton;


    TABLE tokendeposit {  
      name account;
      eosio::asset waxbalance;
  
      auto primary_key() const { return account.value ;}
      EOSLIB_SERIALIZE( tokendeposit, (account)(waxbalance)); 
    };

    typedef eosio::multi_index<name("tokendeposit"), tokendeposit> token_table;

 
};

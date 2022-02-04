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

    ACTION withdraw(const name &user, eosio::asset asset);

    ACTION init(string contract_name, name initial_admin);
    ACTION setadmin(name new_admin);
    ACTION setversion(string new_version);
    ACTION setlockdtime(uint32_t locked_days);
    ACTION addsymbol(symbol_code token_symbol, uint32_t decimals);
    ACTION rmvsymbol(symbol_code token_symbol, uint32_t decimals);


  private:

    TABLE config {
      string contract_name;
      string contract_version;
      name admin_acct;
      uint32_t locked_days;
      vector<symbol_code> accepted_tokens = {};
   
      EOSLIB_SERIALIZE(config, (contract_name)(contract_version)(admin_acct)(locked_days)(accepted_tokens))
    };
    typedef singleton<name("config"), config> config_singleton;

    //scope: self.value
    TABLE users {
      name user;

      auto primary_key() const { return user.value ;}
      EOSLIB_SERIALIZE( users, (user)); 
    };

    typedef eosio::multi_index<name("users"), users> users_table;


    //scope: account.value
    TABLE tokendeposit {
      uint64_t token_id;
      eosio::asset balance;
      symbol_code token;
      time_point_sec deposit_time;
      bool is_locked;

  
      auto primary_key() const { return token_id ;}
      uint64_t by_symbol( ) const { return (uint64_t) token.raw(); }

      EOSLIB_SERIALIZE( tokendeposit, (token_id)(balance)(token)(deposit_time)(is_locked)); 
    };

    typedef eosio::multi_index<name("tokendeposit"), tokendeposit,
      indexed_by<name("symbol"), const_mem_fun<tokendeposit, uint64_t, &tokendeposit::by_symbol>>
      > token_table;

    //scope: self.value
    TABLE tokenstats {
      uint64_t stats_id;
      symbol_code token_symbol;
      uint8_t precision;
      uint64_t total_amount;
  
      auto primary_key() const { return stats_id ;}
      uint64_t by_symbol( ) const { return (uint64_t) token_symbol.raw(); }

      EOSLIB_SERIALIZE( tokenstats, (stats_id)(token_symbol)(precision)(total_amount)); 
    };

    typedef eosio::multi_index<name("tokenstats"), tokenstats,
        indexed_by<name("symbol"), const_mem_fun<tokenstats, uint64_t, &tokenstats::by_symbol>>
        > stats_table;

};

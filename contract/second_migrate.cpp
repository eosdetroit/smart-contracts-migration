void testcontract::migrate(uint16_t counter, uint16_t max_number_rows) {

    //authenticate
    check(has_auth(get_self()), "Missing authority");
    
    migration_table migrations(get_self(), get_self().value);

    if(counter == 0) {
        //Create migration table to keep track of the process. 
        migration _migration;
        _migration.in_process = 2;
        _migration.migrating = true;
        migrations.set(_migration, get_self());
    }

    uint16_t i = 0; //Migrated rows counter


    //Open tables that you are intending to migrate 
    //and their corresponding support table HERE
    /* BEGIN MODIFY */
    support_token_table support_token(get_self(), get_self().value);
    
    support_config_singleton support_configs(get_self(), get_self().value);
    config_singleton configs(get_self(), get_self().value);

    users_table _users(get_self(), get_self().value);

    stats_table _stats(get_self(), get_self().value);

    /* END MODIFY */

    bool migration_ended = false;

    while(i < max_number_rows && !migration_ended) {
        /* BEGIN MODIFY */

        auto token_itr = support_token.begin();
        if(token_itr != support_token.end()) {
            token_table token(get_self(), token_itr->account.value);
                    
            token.emplace(get_self(), [&](auto& col){
                col.balance = token_itr->waxbalance;
                col.token = token_itr->waxbalance.symbol.code();
                col.is_locked = false;
            });

            _users.emplace(get_self(), [&](auto& col){
                col.user = token_itr->account;
            });

            auto stats = _stats.begin();
            if(stats != _stats.end()) {
                _stats.modify(stats, get_self(), [&](auto &col) {
                    col.total_amount += token_itr->waxbalance.amount;
                });
            } else {
                _stats.emplace(get_self(), [&](auto &col) {
                    col.token_symbol = WAX_SYM.code();
                    col.precision = WAX_SYM.precision();
                    col.total_amount = token_itr->waxbalance.amount;
                });
            }

            token_itr = support_token.erase(token_itr);

        /* END MODIFY */
        } else {
            //In the final loop migrate Only singleton Tables
            /* BEGIN MODIFY */
            auto support_conf = support_configs.get();

            if(support_configs.exists()) {
                 config _config;
                _config.contract_name = support_conf.contract_name;
                _config.contract_version = support_conf.contract_version;
                _config.admin_acct = support_conf.admin_acct;
                _config.locked_days = 60;
                _config.accepted_tokens = {WAX_SYM.code()};    

                configs.set(_config, get_self());    
                support_configs.remove();
            }

            /* END MODIFY */

            //Update the migration table by setting that the migration process has ended.       
            migration _updated_migration;
            _updated_migration.in_process = 0;
            _updated_migration.migrating = false;
            migrations.set(_updated_migration, get_self());

            migration_ended = true;
        }
        
        ++i;
    }
}



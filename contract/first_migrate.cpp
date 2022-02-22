void testcontract::migrate(uint16_t counter, uint16_t max_number_rows) {

    //authenticate
    check(has_auth(get_self()), "Missing authority");

    migration_table migrations(get_self(), get_self().value);

    if(counter == 0) {
        //Create migration table to keep track of the process. 
        migration _migration;
        _migration.in_process = 1;
        _migration.migrating = true;
        migrations.set(_migration, get_self());
    }
    
    uint16_t i = 0; //Migrated rows counter

    //Open tables that you are intending to migrate 
    //and their corresponding support table HERE
    /* BEGIN MODIFY */
    token_table token(get_self(), get_self().value);
    support_token_table supporttoken(get_self(), get_self().value);

    config_singleton configs(get_self(), get_self().value);
    support_config_singleton support_configs(get_self(), get_self().value);
    /* END MODIFY */

    bool migration_ended = false;
    
    while (i < max_number_rows && !migration_ended) {
        /* BEGIN MODIFY */

        auto token_itr = token.begin();
        if(token_itr != token.end()) {
            supporttoken.emplace(get_self(), [&](auto& col){
                col.account = token_itr->account;
                col.waxbalance = token_itr->waxbalance;
            });

            token_itr = token.erase(token_itr);

            //If multiple tables need to be migrating, add different else if conditions
            //unless that the second table depends on the first one.

        /* END MODIFY */
        } else {
            //In the final loop migrate Only singleton Tables
            /* BEGIN MODIFY */
            if(configs.exists()) {

                auto conf = configs.get();

                supportconf _config;
                _config.contract_name = conf.contract_name;
                _config.contract_version = conf.contract_version;
                _config.admin_acct = conf.admin_acct;

                support_configs.set(_config, get_self());
                configs.remove();

            }
            /* END MODIFY */

            //Update the migration table by setting that the migration process has ended.       
            migration _updated_migration;
            _updated_migration.in_process = 2;
            _updated_migration.migrating = true;
            migrations.set(_updated_migration, get_self());

            migration_ended = true;
        }

        ++i;
    }
}


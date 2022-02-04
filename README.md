This script intends to automate and simplify migrations in EOSIO Smart Contracts.

Procedure
========================

1.  Create support tables (which means creating a copy) for those tables that are going to be migrated.
2.  Add a temporary migration table that will control when the migration has been completed.
3.  Create and execute first migration action, consisting in copying all data from selected tables to the support ones and also delete it from the original table. Since some tables may have many rows, a maximum of rows will be migrated each time the migration action is called. 
4.  Create and execute second migration action, consisting in moving back all data from the support tables to the original ones. Since those tables have been emptied, we can modify the rows as needed before doing the migration.

At the beginning of the migration,a migration table will be created and a parameter in_process will be set to true. During all the migration process, a check will be automatically added to all the SC actions, so those actions will be frozen until the migration is completed.  Once the migration is completed, the migration table will be removed.

Usage
======================== 

1. Add the original SmartContract files to the old_contract directory. Files must be separated between the include and src directories.

2. Add the new upgraded SmartContract files to the new_contract directory. Files must be separated between the include and src directories.

3. Add the tables that will be migrated in the tablesToMigrate.sh file. Each table to be migrated has to be described in the following format: tableX = (originalTable supportTable originalTableName supportTableName originalTableType supportTableType). Additionally, you should add an extra array containing the name of the variables that contains each table Info.

4. Modify the migrate action in first_migrate.cpp file so the tables that are going to be migrated are copied to the corresponding support tables. In order to work, it is important to use the support table names described in tablesToMigrate.sh

5. Modify migrate action in second_migrate.cpp file so that the rows contained into the support tables are moved back to the original ones with the modifications applied.


Example 
=========================

To try the migration script, an example Smart Contract has been already prepared. The original SC is a simple Smart Contract to stake tokens, while the new one is an advanced version where tokens are locked for a certain period of time.

To reproduce it, one can follow the next steps:

1. Create an account in testnet (one can use the WAXSweden Faucet). 
2. Build and Deploy the SC to that account.
3. Init the SC
4. Do a couple of transfers to prepopulate the Table
5. Run the migrate script
#! /bin/bash

set -e

source helpers/createSupportTables.sh

#contract
if [ -n "$1" ]; then
    contract=$1
else
    echo "need contract"
    exit 0
fi

file_source="src/$contract.cpp"
file_include="include/$contract.hpp"

old_sc_directory="contract/old_contract"
new_sc_directory="contract/new_contract"
file_firstMigrate="contract/first_migrate.cpp"
file_secondMigrate="contract/second_migrate.cpp"

source contract/tablesToMigrate.sh


file_supportTable="helpers/supportTable.cpp"
file_migrateTable="helpers/migrateTable.cpp"
file_checkMigrating="helpers/check_is_migrating.cpp"

rm -rf build
mkdir build

rm -rf src include
rm -rf "$file_supportTable" "$contract.abi" "$contract.wasm"
mkdir src include

if [[ -z "$in_process" || "$in_process" == "1" ]]; then
    cp -r "$old_sc_directory/src/." "./src"
    cp -r "$old_sc_directory/include/." "./include"

    echo ">>> Adding migrate table to $contract.hpp file"
    line=$(grep -n '};' "$file_include" | tail -1 | cut -d : -f 1)
    sed -i "$((line-1)) r $file_migrateTable"  "$file_include"


    echo ">>> Creating $file_supportTable"
    touch $file_supportTable

    for table in "${tables[@]}"
    do  
        params="${table}[@]"
        params_array=("${!params}")
        echo "${params_array[@]}"

        createSupportTable "${params_array[@]}" $file_supportTable "$file_include"
    done


    echo ">>> Adding support tables to $contract.hpp file"
    sed -i '1i\\' $file_supportTable
    line=$(grep -n '};' "$file_include" | tail -1 | cut -d : -f 1)
    sed -i "$((line-1)) r $file_supportTable"  "$file_include"

    echo ">>> Pausing all actions while migrating"
    sed -i -E "/(void|ACTION)(.*)\{/ r $file_checkMigrating" "$file_source"

    echo ">>> Adding first migrate action in $file_source"
    sed -i -E '$a\ \n' "$file_source" 
    cat $file_firstMigrate >> "$file_source"


    echo ">>> Adding first migrate action definition in $file_include"
    if  grep "private:" "$file_include"; then
        sed  -i '/private:/iACTION migrate(uint8_t counter, uint8_t max_number_rows);' "$file_include"
    else
        line=$(grep -n '};' "$file_include" | tail -1 | cut -d : -f 1)
        sed -i "$((line-1)) a ACTION migrate\(uint8\_t counter\, uint8\_t max\_number\_rows\)\;"  "$file_include"
    fi


    #Build
    echo ">>> Building $contract"
    build=$( eosio-cpp -I="./include/" "$file_source" && sleep 5 && wait )

    rm -rf src include
    mkdir src include
fi

cp -r "$new_sc_directory/src/." "./src"
cp -r "$new_sc_directory/include/." "./include"

echo ">>> Adding migrate table to $contract.hpp file"
line=$(grep -n '};' "$file_include" | tail -1 | cut -d : -f 1)
sed -i "$((line-1)) r $file_migrateTable"  "$file_include"

echo ">>> Adding support tables to new SC"
sed -i -E '$a\ \n' "$file_include"
line=$(grep -n '};' "$file_include" | tail -1 | cut -d : -f 1)
sed -i "$((line-1)) r $file_supportTable"  "$file_include"


echo ">>> Pausing all actions while migrating"
sed -i -E "/(void|ACTION)(.*)\{/ r $file_checkMigrating" "$file_source"


echo ">>> Adding second migrate action in $file_source"
sed -i -E '$a\ \n' "$file_source"
cat $file_secondMigrate >> "$file_source"


echo ">>> Adding second migrate action definition in $file_include"
if  grep "private:" "$file_include"; then
    sed  -i '/private:/iACTION migrate(uint8_t counter, uint8_t max_number_rows);' "$file_include"
else
    line=$(grep -n '};' "$file_include" | tail -1 | cut -d : -f 1)
    sed -i "$((line-1)) a ACTION migrate\(uint8\_t counter \, uint8\_t max\_number\_rows\)\;"  "$file_include"
fi


#Build
echo ">>> Building $contract"
build=$( eosio-cpp -I="./include/" "$file_source" && sleep 5 && wait )

rm -rf src include build
rm $file_supportTable 


#Build
echo ">>> Migration completed"
echo ">>> Building upgraded SC without migration features"
build=$( eosio-cpp -I="./$new_sc_directory/include/" "$new_sc_directory/src/$contract.cpp" && sleep 5 && wait )

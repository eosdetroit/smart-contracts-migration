#!/bin/bash

function createSupportTable() {
    if test "$#" -ne 8; then
        echo "Illegal number of parameters"
        exit 0;
    fi  

    table=$1
    supportTable=$2
    tableName=$3
    supportTableName=$4
    tableType=$5
    supportTableType=$6
    file_supportTable=$7
    file_include=$8
    
    echo ">>> Copying $table table to $supportTable table"
    sed -i -E '$a\ \n' $file_supportTable  
    sed -E "/TABLE $table/,/$tableType\s*\;/!d" $file_include >> $file_supportTable
    sed -i "s/TABLE $table/TABLE $supportTable /g" $file_supportTable
    sed -i "s/$table\s*,/$supportTable,/g" $file_supportTable
    sed -i "s/$table\s*:/$supportTable:/g" $file_supportTable
    sed -i "s/$table\s*>/$supportTable>/g" $file_supportTable
    sed -i "s/EOSLIB_SERIALIZE($table/EOSLIB_SERIALIZE($supportTable/g" $file_supportTable
    sed -i "s/name(\"$tableName\")\s*,/name(\"$supportTableName\"),/g" $file_supportTable
    sed -i "s/$tableType/$supportTableType/g" $file_supportTable

}
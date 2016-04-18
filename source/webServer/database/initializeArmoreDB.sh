#!/bin/bash

TMP_FILE="/tmp/dbcreate";

# Create database
DB_NAME="armore.db";
cat /dev/null > $DB_NAME;

# Create Users Table
CREATE_USERS="CREATE TABLE Users (uid INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,username varchar(100) NOT NULL UNIQUE,pwdhash varbinary(256) NOT NULL,roleName varchar(100) NOT NULL DEFAULT \"user\");";
echo $CREATE_USERS > $TMP_FILE;
sqlite3 $DB_NAME < $TMP_FILE;
rm -f $TMP_FILE;

# Create UserRoles Table
CREATE_USERROLES="CREATE TABLE UserRoles(roleName varchar(100) PRIMARY KEY NOT NULL,roleValue int(11) NOT NULL);";
echo $CREATE_USERROLES > $TMP_FILE;
sqlite3 $DB_NAME < $TMP_FILE;
rm -f $TMP_FILE;

# Add Admin and User to UserRoles
ADD_ROLES_USER="INSERT INTO UserRoles (roleName, roleValue) values ('user', 100)";
ADD_ROLES_ADMIN="INSERT INTO UserRoles (roleName, roleValue) values ('admin', 0)";
sqlite3 $DB_NAME "$ADD_ROLES_USER";
sqlite3 $DB_NAME "$ADD_ROLES_ADMIN";

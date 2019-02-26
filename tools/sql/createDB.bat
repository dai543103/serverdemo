echo off

set dbname="battlespheres"	
echo "create database %dbname% begin......."
mysql -uroot -p123456 -hlocalhost -e "CREATE DATABASE IF NOT EXISTS %dbname% DEFAULT CHARSET utf8"
echo "create database %dbname% finish......."

echo "create table from create.sql begin......."
mysql -uroot -p123456 -hlocalhost -D%dbname% < ./create.sql
echo "create table from create.sql finish......."

echo "create produre from produre.sql begin......."
mysql -uroot -p123456 -hlocalhost -D%dbname% < ./update.sql
echo "create produre from produre.sql finish......."	

# Overview

Sections 1 and 2.1 of the Problem Description are implemented by this project in C++.

## Build

1. Clone repository and submodules

  ```
  $ git clone --recursive git@github.com:mutac/query-challenge.git
  ```
  
2. Launch Visual Studio 2013 solution located in query-challenge/ideprojects/
3. Build the solution (this builds Import.exe and Query.exe in query-challenge/examples)

## Usage

1. From a shell, cd to query-challenge/examples
2. Create an initial database using the provided scheme:

  ```
  $ ./Import.exe -c Scheme.json -d db.json
  Database "db.json" created
  ```
  
3. Import some data (if -i is omitted, Import.exe reads from stdin)
  
  ```
  $ ./Import.exe -d db.json -i Example1.txt
  Inserted 4 new rows
  Replaced 0 existing rows
  ```

4. Query the data using Query.exe

  ```
  $ ./Query.exe -d db.json -s TITLE,DATE
  the matrix,2014-04-01
  unbreakable,2014-04-03
  the hobbit,2014-04-02
  the matrix,2014-04-02
  
  $ ./Query.exe -d db.json -s TITLE,DATE,REV -o DATE,REV
  the matrix,2014-04-01,4
  the matrix,2014-04-02,4
  the hobbit,2014-04-02,8
  unbreakable,2014-04-03,6
  
  $ ./Query.exe -d db.json -s TITLE,DATE -o DATE -f REV=4.0
  the matrix,2014-04-01
  the matrix,2014-04-02
  ```

# Problem Description

1. Importer and Datastore

  You will be provided with a pipe separated file in the following format.

  File Sample:
    ```
        STB|TITLE|PROVIDER|DATE|REV|VIEW_TIME
        stb1|the matrix|warner bros|2014-04-01|4.00|1:30
        stb1|unbreakable|buena vista|2014-04-03|6.00|2:05
        stb2|the hobbit|warner bros|2014-04-02|8.00|2:45
        stb3|the matrix|warner bros|2014-04-02|4.00|1:05
    ```
  Field Descriptions:
    ```
        STB - The set top box id on which the media asset was viewed. (Text, max size 64 char)
        TITLE - The title of the media asset. (Text, max size 64 char)
        PROVIDER - The distributor of the media asset. (Text, max size 64 char)
        DATE - The local date on which the content was leased by through the STB (A date in YYYY-MM-DD format)
        REV - The price incurred by the STB to lease the asset. (Price in US dollars and cents)
        VIEW_TIME - The amount of time the STB played the asset.  (Time in hours:seconds)
    ```

    Your first task is to parse and import the file into a simple datastore.  You may use any file format that you want to implement to store the data.  Records in the datastore should be unique by STB, TITLE and DATE.  Subsequent imports with the same logical record should overwrite the earlier records.

2. Query tool

  2.1 select, order and filter

    The next task is to create a simple query tool that can execute simple queries against the datastore you created in step one.  The tool should accept command line args for SELECT, ORDER and FILTER functions something along the lines of:

    ```
        $ ./query -s TITLE,REV,DATE -o DATE,TITLE
    
        the matrix,4.00,2014-04-01
        the hobbit,8.00,2014-04-02
        the matrix,4.00,2014-04-02
        unbreakable,6.00,2014-04-03
    
        $ ./query -s TITLE,REV,DATE -f DATE=2014-04-01
    
        the matrix,4.00,2014-04-01
    ```

  2.2 group and aggregate functions

    The next step is to add group by and aggregate functions to your query tool.  Your tool should support the following aggregates:

    ```
        MIN: select the minimum value from a column
        MAX: select the maximum value from a column
        SUM: select the summation of all values in a column
        COUNT: count the distinct values in a column
        COLLECT: collect the distinct values in a column
    ```

    ```
        $ ./query -s TITLE,REV:sum,STB:collect -g TITLE
    
        the matrix,8.00,[stb1,stb3]
        the hobbit,8.00,[stb2]
        unbreakable,6.00,[stb1]
    ```

  2.3 advanced filter function

    Add a filter function which evaluates boolean AND and OR expressions in the following format:

    ```STB="stb1" AND TITLE="the hobbit" OR TITLE="unbreakable"```

    Assume AND has higher precedence than OR.  Parens can be added to change the above statement to the more logical:

    ```STB="stb1" AND (TITLE="the hobbit" OR TITLE="unbreakable")```

    ```
      $ ./query -s TITLE,REV -f 'TITLE="the hobbit" OR TITLE="the matrix"'
    
    the matrix,4.00
    the hobbit,8.00
    the matrix,4.00
    ```

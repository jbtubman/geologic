# DIMROOM (V2.10)

Jim Tubman
June/July 2025

_A read-only command-line version of Lightroom._

## Installation (Now Uses CMAKE)

Installation instructions are available in the file [INSTALL.md](./INSTALL.md).

## Running Dimroom

To run Dimroom from a terminal window, make sure that you are in the
`geologic/build` directory, then type the command name and the path
to a CSV file, e.g.:

    $ ./dimroom ../test/data/sample.csv

The filename is required. Unfortunately, Dimroom is not very robust if a
non-CSV file is used.

A greeting and the command prompt will appear.

    Welcome to DimRoom
    Enter the command "help" for help.
    dimroom-2.10>

### Getting Help

Very basic help is available.

    dimroom-2.10> help
    "describe" - describe the table
    "query ("column name" operator value)" - do a regular query
            available operators are = != > >= < <=
    "query ("column name" inside (coordinate) (coordinate) (coordinate)...)" - look for coordinates inside a polygon of at least 3 points
    "query ("column name" tags "tag1" "tag2"...)" - look for tags in a column
    "exit" - end program
    "quit" - end program
    "help" - print help message

### Describing The Data

The _describe_ command provides information about the file that was read.

    dimroom-2.10> describe
    Column Name: "Filename"; Column Type : "text"
    Column Name: "Type"; Column Type : "text"
    Column Name: "Image Size (MB)"; Column Type : "floating"
    Column Name: "Image X"; Column Type : "integer"
    Column Name: "Image Y"; Column Type : "integer"
    Column Name: "DPI"; Column Type : "integer"
    Column Name: "(Center) Coordinate"; Column Type : "geo_coordinate"
    Column Name: "Favorite"; Column Type : "boolean"
    Column Name: "Continent"; Column Type : "text"
    Column Name: "Bit color"; Column Type : "integer"
    Column Name: "Alpha"; Column Type : "text"
    Column Name: "Hockey Team"; Column Type : "text"
    Column Name: "User Tags"; Column Type : "tags"

### Running Queries

Simple queries on single columns are support for text, integer, boolean, and
floating point values.

    dimroom-2.10> query ("Type" = "png")
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Iceland.png,png,8.35,600,800,72,,,,,,Team Iceland,"""Johnson, Volcano, Dusk"""
    Italy.png,png,10.5,600,800,96,,1,Europe,,,,
    2 rows found

    dimroom-2.10> query ("Image Size (MB)" = 26.4)
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Japan.jpeg,jpeg,26.4,600,800,600,{ "coordinate" : { "value" : "36° 00' N, 138° 00' E", "format" : "dm" } },,Asia,,,,"""Mt Fuji, Fog"""
    1 rows found

    dimroom-2.10> query ("Image Size (MB)" > 10.0)
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Italy.png,png,10.5,600,800,96,,1,Europe,,,,
    Japan.jpeg,jpeg,26.4,600,800,600,(36° 00' N, 138° 00' E),,Asia,,,,"""Mt Fuji, Fog"""
    Calgary.tif,tiff,30.6,600,800,1200,(51.05011, -114.08529),1,,32,Y,Flames,"""Urban, Dusk"""
    3 rows found

    dimroom-2.10> query ("DPI" = 72)
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Iceland.png,png,8.35,600,800,72,,,,,,Team Iceland,"""Johnson, Volcano, Dusk"""
    Edmonton.jpg,jpeg,5.6,900,400,72,{ "coordinate" : { "value" : "53.55014, -113.46871", "format" : "decimal" } },,,,,Oilers,
    2 rows found

    dimroom-2.10> query ("(Center) Coordinate" = (36° 00' N, 138° 00' E))
    geo_query_match: coord = (36° 00' N, 138° 00' E)
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Japan.jpeg,jpeg,26.4,600,800,600,(36° 00' N, 138° 00' E),,Asia,,,,"""Mt Fuji, Fog"""
    1 rows found

    dimroom-2.10> query ("(Center) Coordinate" inside (60.129, -120.010) (40.742, -120.948) (40.748, -100.867) (60.129, -100.867) )
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Calgary.tif,tiff,30.6,600,800,1200,(51.05011, -114.08529),1,,32,Y,Flames,"""Urban, Dusk"""
    Edmonton.jpg,jpeg,5.6,900,400,72,(53.55014, -113.46871),,,,,Oilers,
    2 rows found

    dimroom-2.10> query ("User Tags" tags "Dusk")
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Iceland.png,png,8.35,600,800,72,,,,,,Team Iceland,"""Johnson, Volcano, Dusk"""
    Calgary.tif,tiff,30.6,600,800,1200,(51.05011, -114.08529),1,,32,Y,Flames,"""Urban, Dusk"""
    2 rows found

    dimroom-2.10> query ("User Tags" tags "Mt Fuji")
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Japan.jpeg,jpeg,26.4,600,800,600,(36° 00' N, 138° 00' E),,Asia,,,,"""Mt Fuji, Fog"""
    1 rows found

    dimroom-2.10> query ("Favorite" = false)
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Iceland.png,png,8.35,600,800,72,,,,,,Team Iceland,"""Johnson, Volcano, Dusk"""
    Japan.jpeg,jpeg,26.4,600,800,600,(36° 00' N, 138° 00' E),,Asia,,,,"""Mt Fuji, Fog"""
    Edmonton.jpg,jpeg,5.6,900,400,72,(53.55014, -113.46871),,,,,Oilers,
    3 rows found

    dimroom-2.10> query ("Favorite" = No)
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Iceland.png,png,8.35,600,800,72,,,,,,Team Iceland,"""Johnson, Volcano, Dusk"""
    Japan.jpeg,jpeg,26.4,600,800,600,(36° 00' N, 138° 00' E),,Asia,,,,"""Mt Fuji, Fog"""
    Edmonton.jpg,jpeg,5.6,900,400,72,(53.55014, -113.46871),,,,,Oilers,
    3 rows found

    dimroom-2.10> query ("Favorite" = yes)
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    Italy.png,png,10.5,600,800,96,,1,Europe,,,,
    Calgary.tif,tiff,30.6,600,800,1200,(51.05011, -114.08529),1,,32,Y,Flames,"""Urban, Dusk"""
    2 rows found

If you search for a non-existent column, you will be told that it is not present.

    dimroom-2.10> query ("Flavour" = "Lemon")
    Column "Flavour" is not in this file.
    Use the "describe" command to see the column names and types.
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    0 rows found

If you search using a data type that is not appropriate for that column, you will get an error message.

    dimroom-2.10> query ("Favorite" 300)
    Error: input 300 for column Favorite was not of type "boolean"
    Filename,Type,Image Size (MB),Image X,Image Y,DPI,(Center) Coordinate,Favorite,Continent,Bit color,Alpha,Hockey Team,User Tags
    0 rows found

#### Unsupported Queries

* **ANDing queries together.** Not yet available. (Internally, lazily evaluated STL range views are
  used for future support of this feature).

### Ending the Program

Typing `exit` or `quit`, or pressing `^D`, will terminate the program.

## Cautionary Notes

### Everything Is In Memory

All the data is kept in memory, so it is an open question about how well this design
will scale when hundreds of thousands of records are used.

### Queries Are Order(_n_)

There is no indexing of any of the in-memory data, so every record is examined for
every query.

A relational database would really be a much better way to handle the back end of
a system like this.

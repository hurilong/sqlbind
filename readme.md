# SqlBind使用教程

> SqlBind是一个轻量级的ORM、代码生成工具，主要是生成与SQL语句与程序语言之间的访问。

## 前言

&emsp;&emsp;在java的世界中orm非常流行，.net我估计也是。而且不止一种框架，我也曾经使用过各种框架，为什么会有这么多的框架？其实每个框架都有自己适应的范围，当你的使用频繁落在框架的范围外的，你就会寻求另外一种框架，周而复始...

&emsp;&emsp;orm框架，大部分都会宣称，不用写SQL就可以完成访问。它们基本上会有一种中间描述，用来对SQL语句的二次封装，小应用还好,但对于复杂的SQL语句，要么就无法支持，要么就特别难写；而且最后转换成的SQL基本上不具有可读性。
而且ORM的对象绑定，如果手写，也是一个不小的工作量。
实际上任何对SQL语句的二次描述，都有一定的学习成本 ，而且也难以表达出SQL语句所能完成的范围，还是最原始的SQL好，对于任何一个程序员来说掌握SQL，基本上是一个必务的要求。

&emsp;&emsp;另一类的方式就是提供统一的接口，如poco、otl，... ，它们提供了大而全的接口，但是他们写SQL语句的时候，常常需要做拼接，这一点让我感到异常难受，一个完整的SQL语句变成几个字符串支离破碎拼接在一起，写起来心里堵。而且调试的时候，只能编译程序后进行调试，一不小心很容易埋下隐患。

&emsp;&emsp;在使用过各种方式后，都没有找到一个让自己满意的。生活还得继续、程序还得编写; 这种折磨多了后，强迫症就起来了。既然没有就自己写一个吧。于是给自己定了一个小小的目标，想达到以下几个要求：
- ORM，能够自动完成映射
  
  在定义数据结构的时候，我会很认真，把数据结构中的成员与表结构中的字段名定义得很相似，在使用的时候希望它能自动通过名称匹配，而我不需要再做映射。当然如果太特别的定义我也能通过映射的方式来实现。手动创建映射关系，是少量的，而大部份是通过自动来完成。

- 原始的SQL语句
    
    我不希望对SQL语句进行二次描述，例如（HSQL）。这样会增加我的学习成本 ，我希望能使用数据库原始的SQL语句。而且二次描述，最终也会翻译成相对应的SQL语句，会有性能消耗。而且希望能支持复杂的SQL语句，什么union,join,子查询等等。

- 希望能一次性写完一整句的SQL
  
  我不想去拼接SQL！拼接的SQL，错误不容易发现。完整写完SQL语句，逻辑上非常清楚，查错也好查。

- 能做一些基本的错误检查
  
  在还没有编译成可执行的代码时，希望能够完成一次错误检查，如表格不存在、字段没有定义、调用错误等等。

- 能方便与程序语言之间完成绑定
    
    希望它能够理解，程序语言中的变量，所处在的位置不同，能自动推导出，是结果集输出，还是参数绑定。

- 希望将来方便扩展到各种语言的输出
  
  目前主要考虑与C++语言的绑定，将来也能够完成，java、.net、php、python
  ...  &nbsp; 之间的绑定，我感觉也它们也遇到同样的问题。

- 数据库移植
  
  希望我能简单替换一下数据库中的SQL，如我原来是使用MYSQL的语句，我换成ORACLE的语句，在代码级别上我就可以完成一次数据库的移植。当然，我要是写成标准的SQL语句，就不用改了。
  
  希望在代码上完成数据库的移植，而不是二进制。因为实际的过程中发现，几乎很少遇到数据库的移植，一开始要考虑到兼容到各种数据库，纯粹浪费时间（个人观点）。只要你一开始把所有的数据库操作，只放在几个代码文件中，而不是穿插在种个业务逻辑的代码中，就已经很不错了。
  
  代码量大的时候，做数据移植基本上不可能，所以我可以接受代码级别的移植，到时候我只要修改这些SQL语句，就完成了一次数据库的移植。

- 能够自动推导
  
  sql 有通配符或默认字段，希望能理解这些能符号的确确含义，并展开。如 * 代表所有字段，我希望传给数据库SQL语句的时候，能把各个字段展开。



&emsp; &emsp; 要支持原始的SQL语句、并且是复杂的SQL语句（无条件限制），注定这项工作，有点大，相当于要编写一个完整的SQL解释器。咬咬牙，终于2016年初始开始动工，年中的时候完成了一个最初工具。

&emsp;&emsp;由于作者使用C++，MySql比较多一些，所这这款工具优先考虑C++与MYSQL之间的绑定，也尝试做一次试验。于是我把这个工具命令为 SqlBind ，主要完成数据库程序语言之间的绑定，数据库之间进行快速访问，减少一些重复的工作，提高开发效率。为了描述的方便，我们下文将SQL生成工具，简称为编译器。

&emsp;&emsp;该文件是以MySql和C++语言的方式编译，其它数据库及程序语言类似参考此文档。
## 工作原理

```mermaid
graph LR
A[描述接口文件]-->|编译|B[C++代码文件]
```

&emsp;&emsp;描述接口文件中，定义了数据结构，表格和访问数据的SQL语句，编译器会解析这些描述，并编译成指定的C\++代码。应用程序就可以在项目中添加这些C++代码文件。

## 接口文件描述

由于作者习惯了C++，所以描述语法跟中C++有些类似。
### 全局属性定义
全局的属性定义如下：

%property_name string 

其中 property_name 为属性名称，string 为属性内容。

#### online 
online 为数据库的属性，定义了当前数据库的连接，对于mysql定义格式如下：

%online "mysql://host=127.0.0.1;user=root;password=123456;database=test;port=3306;ignore_case=false"



参数名称 | 说明
---|---
host | mysql主机地址
user | mysql用户名
password | mysql密码 
database | mysql数据库名称
port | mysql端口号
ignore_case | 取值为 true 或 false, 在导入MYSQL表格结构时是否忽略大小写，true 忽略大小写 , false 区分大小写，默认为false。


#### cpp.header

C++头文件，格式如下：

%cpp.header "test.hpp" 


#### cpp.code
C++代码文件，格式如下：

%cpp_file "test.cpp" 

#### file 
文件输出内容。格式如下： 

%file "test.cpp"  { 

file_content 

}

生成test.cpp 时，会先把 file_contet 的内容输出到该文件中。

#### sql.trace 

在提交给数据库执行输出SQL处理。
sql.trace 应该是一段代码。代码中包含$sql_cmd$的字符字，编译器将实际提交的SQL替换 $sql_cmd$ 。

例如：

```
%sql.trace printf("sqlCmd:%s" , $sql_cmd$);
```

SQL语句为：
```
select * from user 
```

提交数据库运行之前会有：

```
printf("sqlCmd:%s" , "select * from user");
```

如果该属性没有定义，将不会输出sql信息。


#### sql.error
执行SQL出错的时候，输出的信息,信息可以包含两个宏。

宏 | 说明
---|---
${sql.error} | 执行返回的错误信息
${sql.errcode} | 执行返回的错误代码
${file.name} | 描述文件名 
${file.line} | 文件行数


编译内部会使用 ${} 作为宏存在，在日志输出中请尽量避开这些字符，有可能会造成编译器错误。

例如：
```
%sql.error fprintf(stderr,"error_code:%d error_msg:%s" , ${sql.error} , ${sql.errmsg}) ;
```
在mysql中，执行语句错误明，可能输出以下代码

```
fprintf(stderr, "error_code:%d error_msg:%s" , mysql_stmt_errno(stmt) , mysql_stmt_error(stmt)) ;
```
如果属性没有定义，将不会输出错误信息的处理代码。


### 操作命令
操作命令，用于指示，编译器的相关处理

#### include 
包含外部文件，将外部的一些文件包含进行一起处理，用法如下：

```
include "Test.idl" 
```

将Test.idl 包含进来一起处理，实际应用中可以将结构定义和函数分离。


### 结构定义

#### 结构体的定义方式  

```
struct struct_name
{
    type fieldName ; 
    ...
};

type 为数据类型
filedName 字段名称
```

struct 的定义方式参数 thrift 定义方式，请定义成简单的数据格式，不要定义成复合结构。定义的数据类型有以下：


类型 | 定义
---|---
布尔型| bool
8位整数 | byte i8 
16位整数 | short i16
32位整数| int i32 
64位整数| long i64 
浮点类型| float , double 
字符串型 | string 
二进制型 | binary
时间型  | datetime , time , date 
容器 | vector, list , deque , set 


其它说明：
- 编译器将查看 属性 @generate，如果为 false 不生成定义，如果为true(默认为true)则生成定义。


例1：
```
struct UserInfo 
{
    string phone ; 
    int id ; 
    string name ; 
}
```

编译器会在头文件生成C++结构的定义：

```
struct UserInfo 
{
    std::string phone ; 
    int id ; 
    std::string name ; 
}
```


以下的例子，编译器不会在C++中生成结构定义，用户必须自己手动完成定义。

例2：
```
struct UserInfo
{
    @generate=false; 
    string phone ; 
    int id; 
    string name ;
}
```


#### 继承方式定义

继承方式的定义，直接从表结构的若干字段，定义一个数据结构。

- 全表定义：
```
class class_name ( table_name  ) 
{
    type fieldName ; 
    ... 
}
```

在这种定义中，系统会自动

如表格 user 的结构如下：

字段名   | 字段类型| 说明 
---|---|---
id | auto_increment | 数据库自增
name | varchar(100) | 姓名
birthday| date | 生日
phone | varchar(30) | 联系电话
state | int | 用户状态 

定义 UserInfo 如下 

```
class UserInfo : user
{
    datetime updateTime ; 
}

等同于以下定义： 

struct UserInfo
{
    long id ; 
    string name ; 
    date birthday ; 
    string phone ; 
    int state ;  
    datetime updateTime ; 
}

并且 这种方式的定义，编译器会自动记录，结构 UserInfo 与 User 之间的映射关系
```

- 局部定义如下：

```
class UserInfo  : table_name ( field1 [ as aliasName1 ], field2 [ as aliasName2 ] , ... ) 
{
    field_type field_name ; 
    ...
}
``` 
局部定义只取指定的字段生成定义。
如：

```
class UserPhone : user ( id , phone) 
{
}
```

等同于以下定义：
```
struct UserPhone 
{
    long id ; 
    string phone ; 
}
```


- 多表组合定义 

多表组合定义，可以将若干张表组合在一起定义成一个结构。如

```
class UserInfo : user ( sell ) 
{
    
}

class SellInfo  : user(id , name) , sell(product_name,sell_time )
{
    
}
```

- 子结构体集定义 
```
declare UserBaseInfo UserInfo ( id , name )
```
在这种定义方式中，UserBaseInfo 可以为了一个类型定义，但实际过程中，UserBaseInfo 不会出现在程序代码中，在代码中仍然使用 UserInfo 但是只能访问 id , name 这两个成员


### 数据库接口定义 

定义格式如下：

```
interface InterfaceName 
{
    function FunName ( FunParamList ) 
    {
        begin sql 
            SqlCmd ; 
        end ;
    }
}
```

- interface 为保留关键字，说明这是一段接口描述。
- function 为关键字 
- InterfaceName 为用户自行定义
- FunName FunName 为用户自行定义。
- FunParamList 为函数参数列表，格式为 
    type1 name1 , type2 name2 , ... 
- SqlCmd为需要执行的SQL语句 ，可以多行，每个SQL语句之间使用 ';' 分隔。


## SQL 描述

编译器的重点工作在于解释、理解SQL语句。
### 术语
#### 占位符

“占位符”用于描述，与数据库之间的数据传输，有可能是传入的参数，也有可能是传出结果集。格式如下：
```
:VariableName
```

如: 
```
    select id :UserId from user where Phone = :UserPhone ; 
```
这时，UserId 和 UserPhone 为C++ 的变量，编译自动会认为 这条语句会输出 id 并存储在 C++ 变量 UserId 中，使用 :UserPhone 作为查询参数。

#### 转义扩展

转义扩展，是将C++变量转义数据字符串，并作为SQL的一部分传送给数据库，使用格式如下：

```
${VariableName} 
```
如：

```
select Id :UserId from user where LoginName = ${LoginName}
```

如果此时 LoginName 为 "TestUserId" ，最终传送给数据库的的SQL语句是 

```
select Id from user where LoginName='TestuserId' 
```
TestUserId 会被转义SQL相对应的字符串。

#### 原始扩展
原始扩展跟转义扩展不一样，转义会对字符串进行转义，原始扩展不转义，直接替换。使用格式如下：
```
${@VariableName}
VariabelName 为变量名
```

如：

```
select Id :UserId from user where ${@QueryPaam}
```

此时 QueryParam 为 "id = 100 and login_name='hello'"，最终传送给数据库的SQL语句是 
```
select Id from user where id = 100 and login_name= 'hello'
```

原始扩展便于用户自定义，SQL中的where条件中的逻辑判断

#### 通配符展开
SQL中，可以使用 \* 通配符，查询所有字段，编译器不会将 \*，传送给数据库运行，而是将 \* 展开给数据库运行。

例如：user 表有三个字段 id , phone , name 使用下面sql语句时:
```
select * from user
```
编译器会将 * 展开，最给传送给数据库的是：
```
select id,phone,name from user
```

编译器会自动根据环境展开*中的所有字段。


## SQL绑定 

编译器内置一个SQL语法解析器，这个解析器脱离数据存的，它尽量试图去理解，你写的SQL语句（但不运行）。它也尽量去兼容数据库原始的SQL语句。提供了常见的SQL语句。如 SELECT、INSERT,UPDATE ,DELETE 等等。在某些情况下，为了减少工作量，它也有自己的语法糖。下面我们将对这些语法进行描述。


为了方便描述，我们先假定有一表格

用户信息表 user 



```
table user 
{
    id auto_increment ; 
    name varchar(100) ;
    phone varchar(20) ;
    age int ; 
    add_time datetime ; 
}
```

结构体
```
struct UserInfo
{
    long id ; 
    string name ; 
    string phone ; 
    int age ; 
}

```


### select 

通配符的输出：

```
select * :info from tableName where ... 
```

#### 指定字段查询
```

select f1 :info.f1 , f2 :info.f2, ... from table where ... 
```


- 例1

---

单条记录查询 函数如下：
 ```
 function getUserInfo ( UserInfo info ) 
 {
     begin sql 
        select * :info from user where id = :info.id ; 
    end 
 }
 ```
 

返回指定的值
```
function getUserInfo( UserInfo info ) 
{
    begin sql 
        select name :info.name , phone :info.phone from user where id = :info.id
    end ;
}
```

获取总数：

``` 
function getUserCount( int count ) 
{
    begin sql 
        select count(*) :count from user; 
    end ; 
}
```
 

---


#### 返回多条记录

 ```
function getUserInfo ( vector<UserInfo> infos ) 
 {
     begin sql 
        select * :infos from user ; 
    end 
 }
 ```
 
 返回指定值的查询
 
 ```
 function getUserInfo ( vector<UserInfo> infos ) 
 {
	begin sql 
		select name :infos.name ,phone :info.phone from user; 
	end ; 
 }
 ```
 
 
 分页查询 
 
 ```
 function getUserInfo ( vector<UserInfo> infos , int startIndex ,int count )
 {
	begin sql 
		select * :infos from user order by id limit ${startIndex} , ${count}
	end  
 }
 ```
 
 分页查询时请注意，MySql数据库不支持 limit 后面使用参数绑定， 所以需要使用扩展方式参数。
 --- 

#### 没有绑定的SQL处理

在查询语句时，如果查询结果集不被，将被优化处理

例如：

```
function getUserInfo ( UserInfo info ) 
{
    begin sql 
        select * from user ; 
        select id  :info.id , `password` from `user` ;
    end ; 
}
```

由于该查询中 \* , \`password\` 结果集没有被使用，我们
不管user表怎么定义，最终传送给数据库的语句为 

```
select 1 from user;
select id from `user` ;
```

  
 
 ### update 
 
 常规更新 
 ```
 update tableName set f1 = :f1 , f2 =:f2 , ... where ... 
 ```
 常规更指定了，具体需要更新的字段。
 
 对于整体更新可以使用以下方式：
 
 ```
 update tableName @{id,bc,} set :info where ... 
 ```
 整体更新会自动将 info 结构变量跟，tableName 的字段能过名称的配对关联起来。
 
 例
 
 ---
 ```
 function updateUserInfo ( UserInfo info , int id ) 
 {
	begin sql 
		update user set name = :info.name , phone = :info.phone where id = :id ; 	
	end ;
 }
 
 ```
 
 ---
 
 ```
 function updateUserInfo (UserInfo info ) 
 {
	begin sql
		update user set :info where id = :info.id ;
	end ; 
 }
 ```
 
上例是属于整体更新，在该例中，编译器自动会把sql改写成 

```
update user set id = :info.id , name =:info.name , phone =:info.phone where id =:info.id 
```


### insert

insert 使用有以下方式

- 指定列名

```
insert into tableName (f1,f2,...) values (:f1,:f2,...) 
```
在列名上，可以使用 \* 作为例名，\* 代表自动推导，排序之前的定义列  

例如：

```
insert into user ( id , name , *) values ( :id , :name , :infos ) ;
```

编译会自动推导 * 与 :infos 之间的映射关系。



- 使用自动配对字段名的方式

```
insert into tableName @{columnName1, columnName2} value :info
```

其中 @{columnName1, columnName2} 为可选项，如果带上了，是告诉编译器，在进行自动推导的时候，排除掉 columnName1 和 columnName2 这两个列。



例：

---

```

function addUser (UserInfo info) 
{
    begin sql 
        insert into user ( phone , name ) values ( :info.phone , :info.name); 
    end; 
}
```


在上例中，由于 user 中的 id 字段是自增字段，执行上述操作后，编译器会自动生成取回 自增字段的值代码，并存储在 info.id 中。

使用简洁语法，可以使用以下方式

```
function addUser(UserInfo info ) 
{
    begin sql
        insert into user values :info ;
    end ; 
}
```
在这例中，编译器会将 sql 展开成 
```
insert into user (phone,name) values (:info.phone, :info.name)
```

由于 id 是自增字段，上述的方式并不会插入，id字段。如果要插入 id 字段，需加入一属性

```
function  addUser(UserInfo info ) 
{
    begin sql
        insert into user (id , *)  values ( :info.id , :info) ;
    end ; 
}
```

此时，编译器展开的sql 如下：

```
insert into (id,phone,name) values (:info.id , :info.phone , :info.name) ;
```


### delete 

delete 删除语法如下：

```
delete from tableName where ... 
```

## 自动推导

SqlBind 中扩展了 \* 的含义，在insert,update 操作中，\* 也能作为一个列存。

### 推导范围

在表名（如果有别名，则是别名）后面添加 !{ } ，代表自动推导时，跳过该列。如：

```
select * from table !{id , userName} 
```

此时 \* 代表不包含 id , userName 的所有列  

对于 insert 来说

```
insert into table ${id} ( * ) values ( :tableInfo) ; 

```

些时 \* 代表 不包含 id 的所有列。


###  * 自动假期


#### select 

在  select 操作中，\* 的含义跟SQL原始的含义一致。

### insert 

在insert操作中，\* 代表排除已列列的所有列  

如:

```
insert user_info ( id , add_time , * ) values ( :id , :add_time , :user_info) ;
```

此时 * 代表为 排除 id , add_time 的所有列 , 编译器会将这些列跟 :user_info 中的成员配对，如果没有匹配将被最终的语句为

```
insert user_info (id , add_time ) values ( :id , :add_time ) ;
```

如果有一部分配对成功有可能是以下这个样子 

```
insert user_info(id , add_time , user_name, state) values ( :id , :add_time , :user_info.user_name , :user_info.state) ;
```

\* 可以出现在插入列的任何位置，也可以出现多次，如:

```
insert into user_info ( id ,* , add_time , * ) values ( :id , :info1 , now() , :info2 ) 
```

匹配的原则是前面先命中，如果前面命中了，就不会出现在后面中。


- 备注
    在insert操作中，如果字段被定义为 auto_increment ( 自增),在自动推导过程中，如果匹配，会作为返回值，自动赋值，如果你需指明插入类型为 auto_increment 列，你需要手动指定。




### 杂项

### 动态表名 

编译器可以使用动态表格名，使用格式如下：
```
${DynamicTableName} TableName 
```
其中 DynamicTableName 为程序传送的变量，TableName 参考表名，编译器会将这段话理解成 表格${DynamicTableName}和TableName 具有相同的结构.

例如
```
begin getDynamicUser ( vector<UserInfo> infos , string dynamicTableName ) 
{
	begin sql
		select * :infos from ${dynamicTableName} user ;
	end ;
}
```

你也可以使用 ${DynamicTableName , TableName} 来指定 

例如：

```
begin getDynamicUser ( vector<UserInfo> infos , string dynamicTableName ) 
{
	begin sql
		select * :infos from ${dynamicTableName,user} ;
	end ;
}
```

这两者之间的差别在于

${DynamicTableName} TableName 方式 TableName 会出现在sql语句中，而 ${DynamicTableName,TableName} 方式 TableName 不会出现在 sql语句中。


### null 值的处理

对于字符串的null等，在应用程序中，等同于 “” ，对于数值型的 null 值，编译器无法正确处理，这点需要注意。


假定user表的记录如下：


id | phone | name | age  
---|---|---|---
1 | 123456789 | test1 | 10
2 | 123456788 | test2 | null 
3 | 123456787 | test3 | 12 
4 | 123456786 | test4 | null 

函数为 

```
function getUser ( vector<UserInfo> infos ) 
{
    begin sql 
        select * :infos from user; 
    end 
}
```

此时应用执行SQL后获取行的数据就会成为：

id | phone | name | age  
---|---|---|---
1 | 123456789 | test1 | 10
2 | 123456788 | test2 | 10
3 | 123456787 | test3 | 12 
4 | 123456786 | test4 | 12

这是由于null字段并不会引起数据api赋值操作引起的。

为了避免这个问题，应用程序需要假定null为应一个具体的数值。我们需要将函数改写成

```
function getUser ( vector<UserInfo> infos ) 
{
    infos.age = -1 ;
    begin sql
        select *:infos from user ;
    end ;
}
```

或者写成为：

```
function getUser ( vector<UserInfo> infos ) 
{
    begin sql
        select *:infos ,ifnull(age,-1) :infos.age from user ;
    end ;
}
```


这时应用程序得到的数据为

id | phone | name | age  
---|---|---|---
1 | 123456789 | test1 | 10
2 | 123456788 | test2 | -1
3 | 123456787 | test3 | 12 
4 | 123456786 | test4 | -1

## 高级功能

### 存储过程及函数

编译器会根据数据库中的存储过程定义。假定数据库有以下存储过程
```
create procedure testProcedure ( in int id , inout text msgText ) 
```
调用方式如下：

``` 
function testProcedure ( int id , string msgId ) 
{
    begin sql 
        call testProcedure ( :id , :msgId  ) ; 
    end ; 
}
```



### 多条语句
编译器支持多条语句，每条SQL语句之间使用';' 隔开。
例如：

```
function getUser ( vector<UserInfo> infos , int totalCount , int startIndex ,int count )
{
    
    begin sql
        select count(*) :totalCount from user ;         
        select *:infos from user  limit ${startIndex} , ${count} ;
    end ;
}
```


## 后记

&emsp;&emsp; 这款工具，只是我对以往使用SQL的一个总结，我觉得它可以提高我的工作效率，当然我也期待，能够提高你的工作效率。SQL语法分析部分，我是以数据库实际的语法兼容来编写，个人精力有限，我先按照我最常用的一些语句来编写，肯定存在不能覆盖的部分; 希望收到你的反馈，我尽量添加这部分的支持。

网站：http://wwww.sqlbind.cn

QQ群：691083125

email: 3267886@qq.com

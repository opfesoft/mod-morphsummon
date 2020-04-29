# mod-morphsummon

![mod-morphsummon](https://gitlab.com/opfesoft/mod-morphsummon/-/raw/master/icon.png)


## Description

This module allows Warlocks, Death Knights and Mages to alter the appearance of their summoned permanent creatures (incl. the Felguard's weapon). The appropriate model / item IDs can be specified in the configuration file.


## How to use ingame

As GM:
- add NPC permanently:
 ```
 .npc add 601072
 ```
- add NPC temporarily:
 ```
 .npc add temp 601072
 ```


## Installation

Clone Git repository:

```
cd <ACdir>
git clone https://gitlab.com/opfesoft/mod-morphsummon.git modules/mod-morphsummon
```

Import SQL automatically:
```
cd <ACdir>
bash apps/db_assembler/db_assembler.sh
```
choose 8)

Import SQL manually:
```
cd <ACdir>
bash apps/db_assembler/db_assembler.sh
```
choose 4)
```
cd <ACdir>
mysql -P <DBport> -u <DPuser> --password=<DBpassword> world <local/sql/world_custom.sql
```

Without DB Assembler:
```
cd <ACdir>
mysql -P <DBport> -u <DPuser> --password=<DBpassword> world <modules/mod-morphsummon/data/sql/db-world/morphsummon.sql
mysql -P <DBport> -u <DPuser> --password=<DBpassword> characters <modules/mod-morphsummon/data/sql/db-characters/morphsummon_ddl.sql
```


## Edit module configuration (optional)

If you need to change the module configuration, go to your server configuration folder (where your `worldserver` or `worldserver.exe` is), copy `morphsummon.conf.dist` to `morphsummon.conf` and edit that new file.


## Screenshots
![NPC](https://gitlab.com/opfesoft/mod-morphsummon/-/raw/master/images/morphsummon1.jpg "NPC")
![Ghoul](https://gitlab.com/opfesoft/mod-morphsummon/-/raw/master/images/morphsummon2.jpg "Ghoul")
![Felguard](https://gitlab.com/opfesoft/mod-morphsummon/-/raw/master/images/morphsummon3.jpg "Felguard")
![Water Elemental](https://gitlab.com/opfesoft/mod-morphsummon/-/raw/master/images/morphsummon4.jpg "Water Elemental")


## Credits

Stoabrogga: author


## License
This code and content is released under the [GNU AGPL v3](LICENSE.md).

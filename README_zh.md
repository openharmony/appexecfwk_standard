# **用户程序框架子系统**

## 简介

用户程序框架子系统是OpenHarmony为开发者提供的一套开发OpenHarmony应用程序的框架。

**包含以下模块：**

- **AppKit：**是用户程序框架提供给开发者的开发包，开发者基于该开发包可以开发出基于Ability组件的应用。

- **AppManagerService：**应用管理服务，用于管理应用运行关系、调度应用进程生命周期及状态的系统服务。

- **BundleManagerService：**是负责管理安装包的系统服务，常见的比如包安装、更新，卸载和包信息查询等，运行在Foundation进程。

应用程序框架子系统架构如下图所示：

![](figures/appexecfwk.png)



## 目录

```
foundation/appexecfwk/standard
├── kits
│   └── appkit						   # Appkit实现的核心代码
├── common
│   └── log							   # 日志组件目录
├── interfaces
│   └── innerkits					   # 内部接口存放目录
├── services
│   ├── appmgr						   # 用户程序管理服务框架代码
│   └── bundlemgr	                   # 包管理服务框架代码
├── test						       # 测试目录
└── tools                              # bm命令存放目录
```

### 使用说明

#### 根据给定的bundle名称获取ApplicationInfo

* getApplicationInfo参数描述

  | 名称        | 读写属性 | 类型   | 必填 | 描述                                                    |
  | ----------- | -------- | ------ | ---- | ------------------------------------------------------- |
  | bundleName  | 只读     | string | 是   | 应用名                                                  |
  | bundleFlags | 只读     | number | 是   | 0：返回默认app信息<<br/>8：返回包含permissions的app信息 |
  | userId      | 只读     | number | 是   | 用户ID                                                  |
  
* 返回值

  Promise<ApplicationInfo>：返回值为Promise对象，Promise中包含应用信息。

* 示例

```
bundle.getApplicationInfo('com.example.myapplicationInstall', 8, 0).then((data) => {
    console.info("name: for begin");
    console.info("name:" + data.name);
    console.info("bundleName:" + data.bundleName);
    console.info("description:" + data.description);
    console.info("descriptionId:" + data.descriptionId);
    console.info("iconPath:" + data.iconPath);
    console.info("iconId:" + data.iconId);
    console.info("label:" + data.label);
    console.info("labelId:" + data.labelId);
    console.info("deviceId:" + data.deviceId);
    console.info("signatureKey:" + data.signatureKey);
    console.info("process:" + data.process);
    console.info("isSystemApp:" + data.isSystemApp);
    console.info("isLauncherApp:" + data.isLauncherApp);
    console.info("supportedModes:" + data.supportedModes);

    console.info('getApplicationInfo permissions length [' + data.permissions.length + ']');
    for (var j = 0; j < data.permissions.length; j++) {
        console.info("permissions[" + j + "]:" + data.permissions[j]);
    }
    console.info('getApplicationInfo moduleSourceDirs length [' + data.moduleSourceDirs.length + ']');
    for (var j = 0; j < data.moduleSourceDirs.length; j++) {
        console.info("moduleSourceDirs[" + j + "]:" + data.moduleSourceDirs[j]);
    }
    console.info('getApplicationInfo moduleInfos length [' + data.moduleInfos.length + ']');
    for (var j = 0; j < data.moduleInfos.length; j++) {
        console.info("moduleInfos[" + j + "]moduleName:" + data.moduleInfos[j].moduleName);
        console.info("moduleInfos[" + j + "]moduleSourceDir:" + data.moduleInfos[j].moduleSourceDir);
    }
    console.info("entryDir:" + data.entryDir);
    console.info("codePath:" + data.codePath);
    console.info("dataDir:" + data.dataDir);
    console.info("dataBaseDir:" + data.dataBaseDir);
    console.info("cacheDir:" + data.cacheDir);
})
```
根据给定的bundle名称获取ApplicationInfo（callback方式）

* getApplicationInfo参数描述

  | 名称        | 读写属性 | 类型                           | 必填 | 描述                                                    |
  | ----------- | -------- | ------------------------------ | ---- | ------------------------------------------------------- |
  | bundleName  | 只读     | string                         | 是   | 应用名                                                  |
  | bundleFlags | 只读     | number                         | 是   | 0：返回默认app信息<<br/>8：返回包含permissions的app信息 |
  | userId      | 只读     | number                         | 是   | 用户ID                                                  |
  | callback    | 只读     | AsyncCallback<ApplicationInfo> | 是   | 回调方法                                                |

* 返回值

  void

* 示例

```
bundle.getApplicationInfo('com.example.myapplicationInstall', 8, 0, OnReceiveEvent);

function OnReceiveEvent(err, data) {
    console.info("name: for begin");
    console.info("name:" + data.name);
    console.info("bundleName:" + data.bundleName);
    console.info("description:" + data.description);
    console.info("descriptionId:" + data.descriptionId);
    console.info("iconPath:" + data.iconPath);
    console.info("iconId:" + data.iconId);
    console.info("label:" + data.label);
    console.info("labelId:" + data.labelId);
    console.info("deviceId:" + data.deviceId);
    console.info("signatureKey:" + data.signatureKey);
    console.info("process:" + data.process);
    console.info("isSystemApp:" + data.isSystemApp);
    console.info("isLauncherApp:" + data.isLauncherApp);
    console.info("supportedModes:" + data.supportedModes);

    console.info('getApplicationInfo permissions length [' + data.permissions.length + ']');
    for (var j = 0; j < data.permissions.length; j++) {
        console.info("permissions[" + j + "]:" + data.permissions[j]);
    }
    console.info('getApplicationInfo moduleSourceDirs length [' + data.moduleSourceDirs.length + ']');
    for (var j = 0; j < data.moduleSourceDirs.length; j++) {
        console.info("moduleSourceDirs[" + j + "]:" + data.moduleSourceDirs[j]);
    }
    console.info('getApplicationInfo moduleInfos length [' + data.moduleInfos.length + ']');
    for (var j = 0; j < data.moduleInfos.length; j++) {
        console.info("moduleInfos[" + j + "]moduleName:" + data.moduleInfos[j].moduleName);
        console.info("moduleInfos[" + j + "]moduleSourceDir:" + data.moduleInfos[j].moduleSourceDir);
    }
    console.info("entryDir:" + data.entryDir);
    console.info("codePath:" + data.codePath);
    console.info("dataDir:" + data.dataDir);
    console.info("dataBaseDir:" + data.dataBaseDir);
    console.info("cacheDir:" + data.cacheDir);
}
```

#### 获取系统中所有可用的包信息

* getAllBundleInfo参数描述

  | 名称       | 读写属性 | 类型       | 必填 | 描述                                                        |
  | ---------- | -------- | ---------- | ---- | ----------------------------------------------------------- |
  | bundleFlag | 只读     | BundleFlag | 是   | 0：返回默认BundleInfo<br>1：返回包含abilityInfo的BundleInfo |

* 返回值

  Promise<Array<BundleInfo>>：返回值为Promise对象，Promise中包含包信息列表。

* 示例

```
bundle.getAllBundleInfo(0).then((data) => {
    for (var i = 0; i < data.length; i++) {
        console.info("index[" + i + "].name: for begin");
        console.info("index[" + i + "].name:" + data[i].name);
        console.info("index[" + i + "].label:" + data[i].label);
        console.info("index[" + i + "].description:" + data[i].description);
        console.info("index[" + i + "].vendor:" + data[i].vendor);
        console.info("index[" + i + "].versionCode:" + data[i].versionCode);
        console.info("index[" + i + "].versionName:" + data[i].versionName);
        console.info("index[" + i + "].jointUserId:" + data[i].jointUserId);
        console.info("index[" + i + "].minSdkVersion:" + data[i].minSdkVersion);
        console.info("index[" + i + "].maxSdkVersion:" + data[i].maxSdkVersion);
        console.info("index[" + i + "].mainEntry:" + data[i].mainEntry);
        console.info("index[" + i + "].cpuAbi:" + data[i].cpuAbi);
        console.info("index[" + i + "].appId:" + data[i].appId);
        console.info("index[" + i + "].compatibleVersion:" + data[i].compatibleVersion);
        console.info("index[" + i + "].targetVersion:" + data[i].targetVersion);
        console.info("index[" + i + "].releaseType:" + data[i].releaseType);
        console.info("index[" + i + "].uid:" + data[i].uid);
        console.info("index[" + i + "].gid:" + data[i].gid);
        console.info("index[" + i + "].seInfo:" + data[i].seInfo);
        console.info("index[" + i + "].entryModuleName:" + data[i].entryModuleName);
        console.info("index[" + i + "].isKeepAlive:" + data[i].isKeepAlive);
        console.info("index[" + i + "].isNativeApp:" + data[i].isNativeApp);
        console.info("index[" + i + "].installTime:" + data[i].installTime);
        console.info("index[" + i + "].updateTime:" + data[i].updateTime);
        console.info("index[" + i + "].appInfo.name:" + data[i].applicationInfo.name);
        console.info("index[" + i + "].appInfo.bundleName:" + data[i].applicationInfo.bundleName);
        console.info('getAllBundleInfo reqPermissions length [' + data[i].reqPermissions.length + ']');
        for (var j = 0; j < data[i].reqPermissions.length; j++) {
            console.info("index[" + i + "]reqPermissions[" + j + "]:" + data[i].reqPermissions[j]);
        }
        console.info('getAllBundleInfo defPermissions length [' + data[i].defPermissions.length + ']');
        for (var j = 0; j < data[i].defPermissions.length; j++) {
            console.info("index[" + i + "]defPermissions[" + j + "]:" + data[i].defPermissions[j]);
        }

        console.info('getAllBundleInfo hapModuleNames length [' + data[i].hapModuleNames.length + ']');
        for (var j = 0; j < data[i].hapModuleNames.length; j++) {
            console.info("index[" + i + "]hapModuleNames[" + j + "]:" + data[i].hapModuleNames[j]);
        }
        console.info('getAllBundleInfo moduleNames length [' + data[i].moduleNames.length + ']');
        for (var j = 0; j < data[i].moduleNames.length; j++) {
            console.info("index[" + i + "]moduleNames[" + j + "]:" + data[i].moduleNames[j]);
        }
        console.info('getAllBundleInfo modulePublicDirs length [' + data[i].modulePublicDirs.length + ']');
        for (var j = 0; j < data[i].modulePublicDirs.length; j++) {
            console.info("index[" + i + "]modulePublicDirs[" + j + "]:" + data[i].modulePublicDirs[j]);
        }
        console.info('getAllBundleInfo moduleDirs length [' + data[i].moduleDirs.length + ']');
        for (var j = 0; j < data[i].moduleDirs.length; j++) {
            console.info("index[" + i + "]moduleDirs[" + j + "]:" + data[i].moduleDirs[j]);
        }
        console.info('getAllBundleInfo moduleResPaths length [' + data[i].moduleResPaths.length + ']');
        for (var j = 0; j < data[i].moduleResPaths.length; j++) {
            console.info("index[" + i + "]moduleResPaths[" + j + "]:" + data[i].moduleResPaths[j]);
        }
        console.info('getAllBundleInfo abilityInfo length [' + data[i].abilityInfos.length + ']');
        for (var j = 0; j < data[i].abilityInfos.length; j++) {
            console.info("index[" + i + "]abilityInfos[" + j + "]name:" + data[i].abilityInfos[j].name);
            console.info("index[" + i + "]abilityInfos[" + j + "]package:" + data[i].abilityInfos[j].package);
        }
    }
})
```

获取系统中所有可用的包信息（callback形式）

* getAllBundleInfo参数描述

  | 名称       | 读写属性 | 类型                             | 必填 | 描述                                                         |
  | ---------- | -------- | -------------------------------- | ---- | ------------------------------------------------------------ |
  | bundleFlag | 只读     | BundleFlag                       | 是   | 0：返回默认BundleInfo<br/>1：返回包含abilityInfo的BundleInfo |
  | callback   | 只读     | AsyncCallback<Array<BundleInfo>> | 是   | 回调方法                                                     |

* 返回值

  void

* 示例

```
bundle.getAllBundleInfo(0, OnReceiveEvent);

function OnReceiveEvent(err, data) {
    console.info('xxx getAllBundleInfo data length [' + data.length + ']');
    for (var i = 0; i < data.length; i++) {
        console.info("index[" + i + "].name: for begin");
        console.info("index[" + i + "].name:" + data[i].name);
        console.info("index[" + i + "].label:" + data[i].label);
        console.info("index[" + i + "].description:" + data[i].description);
        console.info("index[" + i + "].vendor:" + data[i].vendor);
        console.info("index[" + i + "].versionCode:" + data[i].versionCode);
        console.info("index[" + i + "].versionName:" + data[i].versionName);
        console.info("index[" + i + "].jointUserId:" + data[i].jointUserId);
        console.info("index[" + i + "].minSdkVersion:" + data[i].minSdkVersion);
        console.info("index[" + i + "].maxSdkVersion:" + data[i].maxSdkVersion);
        console.info("index[" + i + "].mainEntry:" + data[i].mainEntry);
        console.info("index[" + i + "].cpuAbi:" + data[i].cpuAbi);
        console.info("index[" + i + "].appId:" + data[i].appId);
        console.info("index[" + i + "].compatibleVersion:" + data[i].compatibleVersion);
        console.info("index[" + i + "].targetVersion:" + data[i].targetVersion);
        console.info("index[" + i + "].releaseType:" + data[i].releaseType);
        console.info("index[" + i + "].uid:" + data[i].uid);
        console.info("index[" + i + "].gid:" + data[i].gid);
        console.info("index[" + i + "].seInfo:" + data[i].seInfo);
        console.info("index[" + i + "].entryModuleName:" + data[i].entryModuleName);
        console.info("index[" + i + "].isKeepAlive:" + data[i].isKeepAlive);
        console.info("index[" + i + "].isNativeApp:" + data[i].isNativeApp);
        console.info("index[" + i + "].installTime:" + data[i].installTime);
        console.info("index[" + i + "].updateTime:" + data[i].updateTime);
        console.info("index[" + i + "].appInfo.name:" + data[i].applicationInfo.name);
        console.info("index[" + i + "].appInfo.bundleName:" + data[i].applicationInfo.bundleName);
        console.info('getAllBundleInfo reqPermissions length [' + data[i].reqPermissions.length + ']');
        for (var j = 0; j < data[i].reqPermissions.length; j++) {
            console.info("index[" + i + "]reqPermissions[" + j + "]:" + data[i].reqPermissions[j]);
        }
        console.info('getAllBundleInfo defPermissions length [' + data[i].defPermissions.length + ']');
        for (var j = 0; j < data[i].defPermissions.length; j++) {
            console.info("index[" + i + "]defPermissions[" + j + "]:" + data[i].defPermissions[j]);
        }

        console.info('getAllBundleInfo hapModuleNames length [' + data[i].hapModuleNames.length + ']');
        for (var j = 0; j < data[i].hapModuleNames.length; j++) {
            console.info("index[" + i + "]hapModuleNames[" + j + "]:" + data[i].hapModuleNames[j]);
        }
        console.info('getAllBundleInfo moduleNames length [' + data[i].moduleNames.length + ']');
        for (var j = 0; j < data[i].moduleNames.length; j++) {
            console.info("index[" + i + "]moduleNames[" + j + "]:" + data[i].moduleNames[j]);
        }
        console.info('getAllBundleInfo modulePublicDirs length [' + data[i].modulePublicDirs.length + ']');
        for (var j = 0; j < data[i].modulePublicDirs.length; j++) {
            console.info("index[" + i + "]modulePublicDirs[" + j + "]:" + data[i].modulePublicDirs[j]);
        }
        console.info('getAllBundleInfo moduleDirs length [' + data[i].moduleDirs.length + ']');
        for (var j = 0; j < data[i].moduleDirs.length; j++) {
            console.info("index[" + i + "]moduleDirs[" + j + "]:" + data[i].moduleDirs[j]);
        }
        console.info('getAllBundleInfo moduleResPaths length [' + data[i].moduleResPaths.length + ']');
        for (var j = 0; j < data[i].moduleResPaths.length; j++) {
            console.info("index[" + i + "]moduleResPaths[" + j + "]:" + data[i].moduleResPaths[j]);
        }
        console.info('getAllBundleInfo abilityInfo length [' + data[i].abilityInfos.length + ']');
        for (var j = 0; j < data[i].abilityInfos.length; j++) {
            console.info("index[" + i + "]abilityInfos[" + j + "]name:" + data[i].abilityInfos[j].name);
            console.info("index[" + i + "]abilityInfos[" + j + "]package:" + data[i].abilityInfos[j].package);
        }
    }
}
```

#### 根据bundle名称获取BundleInfo

* getBundleInfo参数描述

  | 名称        | 读写属性 | 类型   | 必填 | 描述                                                         |
  | ----------- | -------- | ------ | ---- | ------------------------------------------------------------ |
  | bundleName  | 只读     | string | 是   | 包名                                                         |
  | bundleFlags | 只读     | number | 是   | 0：返回默认BundleInfo<br/>1：返回包含abilityInfo的BundleInfo |

* 返回值

  Promise<BundleInfo>：返回值为Promise对象，Promise中包含包信息。

* 示例

```
bundle.getBundleInfo('com.example.myapplicationInstall', 1).then((data) => {
    console.info("name:" + data.name);
    console.info("label:" + data.label);
    console.info("description:" + data.description);
    console.info("vendor:" + data.vendor);
    console.info("versionCode:" + data.versionCode);
    console.info("versionName:" + data.versionName);
    console.info("jointUserId:" + data.jointUserId);
    console.info("minSdkVersion:" + data.minSdkVersion);
    console.info("maxSdkVersion:" + data.maxSdkVersion);
    console.info("mainEntry:" + data.mainEntry);
    console.info("cpuAbi:" + data.cpuAbi);
    console.info("appId:" + data.appId);
    console.info("compatibleVersion:" + data.compatibleVersion);
    console.info("targetVersion:" + data.targetVersion);
    console.info("releaseType:" + data.releaseType);
    console.info("uid:" + data.uid);
    console.info("gid:" + data.gid);
    console.info("seInfo:" + data.seInfo);
    console.info("entryModuleName:" + data.entryModuleName);
    console.info("isKeepAlive:" + data.isKeepAlive);
    console.info("isNativeApp:" + data.isNativeApp);
    console.info("installTime:" + data.installTime);
    console.info("updateTime:" + data.updateTime);
    console.info("appInfo.name:" + data.applicationInfo.name);
    console.info("appInfo.bundleName:" + data.applicationInfo.bundleName);
    console.info('getBundleInfo reqPermissions length [' + data.reqPermissions.length + ']');
    for (var j = 0; j < data.reqPermissions.length; j++) {
        console.info("reqPermissions[" + j + "]:" + data.reqPermissions[j]);
    }
    console.info('getBundleInfo defPermissions length [' + data.defPermissions.length + ']');
    for (var j = 0; j < data.defPermissions.length; j++) {
        console.info("defPermissions[" + j + "]:" + data.defPermissions[j]);
    }

    console.info('getBundleInfo hapModuleNames length [' + data.hapModuleNames.length + ']');
    for (var j = 0; j < data.hapModuleNames.length; j++) {
        console.info("hapModuleNames[" + j + "]:" + data.hapModuleNames[j]);
    }
    console.info('getBundleInfo moduleNames length [' + data.moduleNames.length + ']');
    for (var j = 0; j < data.moduleNames.length; j++) {
        console.info("moduleNames[" + j + "]:" + data.moduleNames[j]);
    }
    console.info('getBundleInfo modulePublicDirs length [' + data.modulePublicDirs.length + ']');
    for (var j = 0; j < data.modulePublicDirs.length; j++) {
        console.info("modulePublicDirs[" + j + "]:" + data.modulePublicDirs[j]);
    }
    console.info('getBundleInfo moduleDirs length [' + data.moduleDirs.length + ']');
    for (var j = 0; j < data.moduleDirs.length; j++) {
        console.info("moduleDirs[" + j + "]:" + data.moduleDirs[j]);
    }
    console.info('getBundleInfo moduleResPaths length [' + data.moduleResPaths.length + ']');
    for (var j = 0; j < data.moduleResPaths.length; j++) {
        console.info("moduleResPaths[" + j + "]:" + data.moduleResPaths[j]);
    }
    console.info('getBundleInfo abilityInfo length [' + data.abilityInfos.length + ']');
    for (var j = 0; j < data.abilityInfos.length; j++) {
        console.info("abilityInfos[" + j + "]name:" + data.abilityInfos[j].name);
        console.info("abilityInfos[" + j + "]package:" + data.abilityInfos[j].package);
    }
})
```

根据bundle名称获取BundleInfo（callback形式）

* getBundleInfo参数描述

  | 名称        | 读写属性 | 类型                      | 必填 | 描述                                                         |
  | ----------- | -------- | ------------------------- | ---- | ------------------------------------------------------------ |
  | bundleName  | 只读     | string                    | 是   | 包名                                                         |
  | bundleFlags | 只读     | number                    | 是   | 0：返回默认BundleInfo<br/>1：返回包含abilityInfo的BundleInfo |
  | callback    | 只读     | AsyncCallback<BundleInfo> | 是   | 回调方法                                                     |

* 返回值

  void

* 示例

```
bundle.getBundleInfo('com.example.myapplicationInstall', 1, OnReceiveEvent);

function OnReceiveEvent(err, data) {
    console.info("name:" + data.name);
    console.info("label:" + data.label);
    console.info("description:" + data.description);
    console.info("vendor:" + data.vendor);
    console.info("versionCode:" + data.versionCode);
    console.info("versionName:" + data.versionName);
    console.info("jointUserId:" + data.jointUserId);
    console.info("minSdkVersion:" + data.minSdkVersion);
    console.info("maxSdkVersion:" + data.maxSdkVersion);
    console.info("mainEntry:" + data.mainEntry);
    console.info("cpuAbi:" + data.cpuAbi);
    console.info("appId:" + data.appId);
    console.info("compatibleVersion:" + data.compatibleVersion);
    console.info("targetVersion:" + data.targetVersion);
    console.info("releaseType:" + data.releaseType);
    console.info("uid:" + data.uid);
    console.info("gid:" + data.gid);
    console.info("seInfo:" + data.seInfo);
    console.info("entryModuleName:" + data.entryModuleName);
    console.info("isKeepAlive:" + data.isKeepAlive);
    console.info("isNativeApp:" + data.isNativeApp);
    console.info("installTime:" + data.installTime);
    console.info("updateTime:" + data.updateTime);
    console.info("appInfo.name:" + data.applicationInfo.name);
    console.info("appInfo.bundleName:" + data.applicationInfo.bundleName);
    console.info('getBundleInfo reqPermissions length [' + data.reqPermissions.length + ']');
    for (var j = 0; j < data.reqPermissions.length; j++) {
        console.info("reqPermissions[" + j + "]:" + data.reqPermissions[j]);
    }
    console.info('getBundleInfo defPermissions length [' + data.defPermissions.length + ']');
    for (var j = 0; j < data.defPermissions.length; j++) {
        console.info("defPermissions[" + j + "]:" + data.defPermissions[j]);
    }

    console.info('getBundleInfo hapModuleNames length [' + data.hapModuleNames.length + ']');
    for (var j = 0; j < data.hapModuleNames.length; j++) {
        console.info("hapModuleNames[" + j + "]:" + data.hapModuleNames[j]);
    }
    console.info('getBundleInfo moduleNames length [' + data.moduleNames.length + ']');
    for (var j = 0; j < data.moduleNames.length; j++) {
        console.info("moduleNames[" + j + "]:" + data.moduleNames[j]);
    }
    console.info('getBundleInfo modulePublicDirs length [' + data.modulePublicDirs.length + ']');
    for (var j = 0; j < data.modulePublicDirs.length; j++) {
        console.info("modulePublicDirs[" + j + "]:" + data.modulePublicDirs[j]);
    }
    console.info('getBundleInfo moduleDirs length [' + data.moduleDirs.length + ']');
    for (var j = 0; j < data.moduleDirs.length; j++) {
        console.info("moduleDirs[" + j + "]:" + data.moduleDirs[j]);
    }
    console.info('getBundleInfo moduleResPaths length [' + data.moduleResPaths.length + ']');
    for (var j = 0; j < data.moduleResPaths.length; j++) {
        console.info("moduleResPaths[" + j + "]:" + data.moduleResPaths[j]);
    }
    console.info('getBundleInfo abilityInfo length [' + data.abilityInfos.length + ']');
    for (var j = 0; j < data.abilityInfos.length; j++) {
        console.info("abilityInfos[" + j + "]name:" + data.abilityInfos[j].name);
        console.info("abilityInfos[" + j + "]package:" + data.abilityInfos[j].package);
    }
}
```

#### 获取指定用户下所有已安装的应用信息

* getAllApplicationInfo参数描述

  | 名称        | 读写属性 | 类型   | 必填 | 描述                                                    |
  | ----------- | -------- | ------ | ---- | ------------------------------------------------------- |
  | bundleFlags | 只读     | number | 是   | 0：返回默认app信息<<br/>8：返回包含permissions的app信息 |
  | userId      | 只读     | number | 是   | 用户ID                                                  |

* 返回值

  Promise<Array<ApplicationInfo>>：返回值为Promise对象，Promise中包含应用信息列表。

* 示例

```
bundle.getAllApplicationInfo(8, 0).then((data) => {
    console.info('xxx getAllApplicationInfo data length [' + data.length + ']');
    for (var i = 0; i < data.length; i++) {
        console.info("index[" + i + "].name: for begin");
        console.info("index[" + i + "].name:" + data[i].name);
        console.info("index[" + i + "].bundleName:" + data[i].bundleName);
        console.info("index[" + i + "].description:" + data[i].description);
        console.info("index[" + i + "].descriptionId:" + data[i].descriptionId);
        console.info("index[" + i + "].iconPath:" + data[i].iconPath);
        console.info("index[" + i + "].iconId:" + data[i].iconId);
        console.info("index[" + i + "].label:" + data[i].label);
        console.info("index[" + i + "].labelId:" + data[i].labelId);
        console.info("index[" + i + "].deviceId:" + data[i].deviceId);
        console.info("index[" + i + "].signatureKey:" + data[i].signatureKey);
        console.info("index[" + i + "].process:" + data[i].process);
        console.info("index[" + i + "].isSystemApp:" + data[i].isSystemApp);
        console.info("index[" + i + "].isLauncherApp:" + data[i].isLauncherApp);
        console.info("index[" + i + "].supportedModes:" + data[i].supportedModes);

        console.info('getAllApplicationInfo Async permissions length [' + data[i].permissions.length + ']');
        for (var j = 0; j < data[i].permissions.length; j++) {
            console.info("index[" + i + "]permissions[" + j + "]:" + data[i].permissions[j]);
        }
        console.info('getAllApplicationInfo Async moduleSourceDirs length [' + data[i].moduleSourceDirs.length + ']');
        for (var j = 0; j < data[i].moduleSourceDirs.length; j++) {
            console.info("index[" + i + "]moduleSourceDirs[" + j + "]:" + data[i].moduleSourceDirs[j]);
        }
        console.info('getAllApplicationInfo Async moduleInfos length [' + data[i].moduleInfos.length + ']');
        for (var j = 0; j < data[i].moduleInfos.length; j++) {
            console.info("index[" + i + "]moduleInfos[" + j + "]moduleName:" + data[i].moduleInfos[j].moduleName);
            console.info("index[" + i + "]moduleInfos[" + j + "]moduleSourceDir:" + data[i].moduleInfos[j].moduleSourceDir);
        }
        console.info("index[" + i + "].entryDir:" + data[i].entryDir);
        console.info("index[" + i + "].codePath:" + data[i].codePath);
        console.info("index[" + i + "].dataDir:" + data[i].dataDir);
        console.info("index[" + i + "].dataBaseDir:" + data[i].dataBaseDir);
        console.info("index[" + i + "].cacheDir:" + data[i].cacheDir);
    }
})
```

获取指定用户下所有已安装的应用信息（callback形式）

* getAllApplicationInfo参数描述

  | 名称        | 读写属性 | 类型                                  | 必填 | 描述                                                    |
  | ----------- | -------- | ------------------------------------- | ---- | ------------------------------------------------------- |
  | bundleFlags | 只读     | number                                | 是   | 0：返回默认app信息<<br/>8：返回包含permissions的app信息 |
  | userId      | 只读     | number                                | 是   | 用户ID                                                  |
  | callback    | 只读     | AsyncCallback<Array<ApplicationInfo>> | 是   | 回调方法                                                |

* 返回值

  void

* 示例

```
bundle.getAllApplicationInfo(8, 0, OnReceiveEvent);

function OnReceiveEvent(err, data) {
    console.info('xxx getAllApplicationInfo data length [' + data.length + ']');
    for (var i = 0; i < data.length; i++) {
        console.info("index[" + i + "].name: for begin");
        console.info("index[" + i + "].name:" + data[i].name);
        console.info("index[" + i + "].bundleName:" + data[i].bundleName);
        console.info("index[" + i + "].description:" + data[i].description);
        console.info("index[" + i + "].descriptionId:" + data[i].descriptionId);
        console.info("index[" + i + "].iconPath:" + data[i].iconPath);
        console.info("index[" + i + "].iconId:" + data[i].iconId);
        console.info("index[" + i + "].label:" + data[i].label);
        console.info("index[" + i + "].labelId:" + data[i].labelId);
        console.info("index[" + i + "].deviceId:" + data[i].deviceId);
        console.info("index[" + i + "].signatureKey:" + data[i].signatureKey);
        console.info("index[" + i + "].process:" + data[i].process);
        console.info("index[" + i + "].isSystemApp:" + data[i].isSystemApp);
        console.info("index[" + i + "].isLauncherApp:" + data[i].isLauncherApp);
        console.info("index[" + i + "].supportedModes:" + data[i].supportedModes);

        console.info('getAllApplicationInfo Async permissions length [' + data[i].permissions.length + ']');
        for (var j = 0; j < data[i].permissions.length; j++) {
            console.info("index[" + i + "]permissions[" + j + "]:" + data[i].permissions[j]);
        }
        console.info('getAllApplicationInfo Async moduleSourceDirs length [' + data[i].moduleSourceDirs.length + ']');
        for (var j = 0; j < data[i].moduleSourceDirs.length; j++) {
            console.info("index[" + i + "]moduleSourceDirs[" + j + "]:" + data[i].moduleSourceDirs[j]);
        }
        console.info('getAllApplicationInfo Async moduleInfos length [' + data[i].moduleInfos.length + ']');
        for (var j = 0; j < data[i].moduleInfos.length; j++) {
            console.info("index[" + i + "]moduleInfos[" + j + "]moduleName:" + data[i].moduleInfos[j].moduleName);
            console.info("index[" + i + "]moduleInfos[" + j + "]moduleSourceDir:" + data[i].moduleInfos[j].moduleSourceDir);
        }
        console.info("index[" + i + "].entryDir:" + data[i].entryDir);
        console.info("index[" + i + "].codePath:" + data[i].codePath);
        console.info("index[" + i + "].dataDir:" + data[i].dataDir);
        console.info("index[" + i + "].dataBaseDir:" + data[i].dataBaseDir);
        console.info("index[" + i + "].cacheDir:" + data[i].cacheDir);
    }
}
```


#### 通过Want获取对应的Ability信息

* queryAbilityInfo参数描述

  | 名称        | 读写属性 | 类型   | 必填 | 描述                                                         |
  | ----------- | -------- | ------ | ---- | ------------------------------------------------------------ |
  | want        | 只读     | Want   | 是   | 指定Want信息                                                 |
  | bundleFlags | 只读     | number | 是   | 0：返回默认BundleInfo<br/>1：返回包含abilityInfo的BundleInfo |
  | userId      | 只读     | number | 是   | 用户ID                                                       |

* Want类型说明

  | 名称        | 读写属性 | 类型                 | 必填 | 描述                                                         |
  | ----------- | -------- | -------------------- | ---- | ------------------------------------------------------------ |
  | elementName | 只读     | ElementName          | 是   | 表示运行指定Ability的ElementName。                           |
  | uri         | 只读     | string               | 否   | 表示Uri描述。                                                |
  | flags       | 只读     | int                  | 否   | Ability的flag，表示处理Want的方式。                          |
  | type        | 只读     | string               | 否   | Want中的type属性是指由Want的URI所指示的资源的MIME类型。      |
  | action      | 只读     | string               | 否   | 表示动作，通常使用系统预置Action，应用也可以自定义Action。   |
  | want_param  | 只读     | {[key: string]: any} | 否   | want_param是一种支持自定义的数据结构，开发者可以通过want_param传递某些请求所需的额外信息。 |
  | entities    | 只读     | Array<string>        | 否   | 表示类别，通常使用系统预置Entity，应用也可以自定义Entity。   |

* ElementName类型说明

  | 名称        | 读写属性 | 类型   | 必填 | 描述                                                         |
  | ----------- | -------- | ------ | ---- | ------------------------------------------------------------ |
  | deviceId    | 只读     | string | 否   | 表示运行指定Ability的设备ID。                                |
  | bundleName  | 只读     | string | 是   | 表示包描述。如果在Want中同时指定了BundleName和AbilityName，则Want可以直接匹配到指定的Ability。 |
  | abilityName | 只读     | string | 是   | 表示待启动的Ability名称。如果在Want中同时指定了BundleName和AbilityName，则Want可以直接匹配到指定的Ability。 |

* 返回值

  Promise<AbilityInfo>：返回值为Promise对象，Promise中包含Ability信息。

* 示例

```
bundle.queryAbilityByWant({
    want: {
        action: "action.system.home",
        entities: ["entity.system.home"],
        elementName: {
            deviceId: "0",
            bundleName: "com.example.myapplicationInstall",
            abilityName: "com.example.myapplication.MainAbility",
        },
    }
},  1, 0,
}).then((data) => {
    console.info("name:" + data.name);
    console.info("label:" + data.label);
    console.info("description:" + data.description);
    console.info("iconPath:" + data.iconPath);
    console.info("visible:" + data.visible);
    console.info("kind:" + data.kind);
    console.info("uri:" + data.uri);
    console.info("process:" + data.process);
    console.info("package:" + data.package);
    console.info("bundleName:" + data.bundleName);
    console.info("moduleName:" + data.moduleName);
    console.info("applicationName:" + data.applicationName);
    console.info("deviceId:" + data.deviceId);
    console.info("codePath:" + data.codePath);
    console.info("resourcePath:" + data.resourcePath);
    console.info("libPath:" + data.libPath);

    console.info('queryAbilityInfo permissions length [' + data.permissions.length + ']');
    for (var j = 0; j < data.permissions.length; j++) {
        console.info("permissions[" + j + "]:" + data.permissions[j]);
    }
    console.info('queryAbilityInfo deviceTypes length [' + data.deviceTypes.length + ']');
    for (var j = 0; j < data.deviceTypes.length; j++) {
        console.info("deviceTypes[" + j + "]:" + data.deviceTypes[j]);
    }
    console.info('queryAbilityInfo deviceCapabilities length [' + data.deviceCapabilities.length + ']');
    for (var j = 0; j < data.deviceCapabilities.length; j++) {
        console.info("deviceCapabilities[" + j + "]:" + data.deviceCapabilities[j]);
    }
    console.info("appInfo.name:" + data.applicationInfo.name);
    console.info("appInfo.bundleName:" + data.applicationInfo.bundleName);
    // ability type: 0:UNKNOWN, 1:PAGE, 2:SERVICE, 3:DATA
    console.info("type:" + data.type);
    // orientation: 0:UNSPECIFIED, 1:LANDSCAPE, 2:PORTRAIT, 3:FOLLOWRECENT,
    console.info("orientation:" + data.orientation);
    // launchMode: 0:SINGLETON, 1:SINGLETOP, 2:STANDARD
    console.info("launchMode:" + data.launchMode);

    // the enum of AbilityType
    console.info("AbilityType:" + bundle.AbilityType.UNKNOWN);
    console.info("AbilityType:" + bundle.AbilityType.PAGE);
    console.info("AbilityType:" + bundle.AbilityType.SERVICE);
    console.info("AbilityType:" + bundle.AbilityType.DATA);
    if (data.type == bundle.AbilityType.PAGE) {
        console.info("this AbilityType is PAGE");
    }
    // the enum of DisplayOrientation
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.UNSPECIFIED);
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.LANDSCAPE);
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.PORTRAIT);
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.FOLLOWRECENT);
    if (data.orientation == bundle.DisplayOrientation.UNSPECIFIED) {
        console.info("this DisplayOrientation is UNSPECIFIED");
    }
    // the enum of LaunchMode
    console.info("LaunchMode:" + bundle.LaunchMode.SINGLETON);
    console.info("LaunchMode:" + bundle.LaunchMode.SINGLETOP);
    console.info("LaunchMode:" + bundle.LaunchMode.STANDARD);
    if (data.launchMode == bundle.LaunchMode.STANDARD) {
        console.info("this LaunchMode is STANDARD");
    }

})
```

通过Want获取对应的Ability信息（callback形式）

* queryAbilityInfo参数描述

  | 名称        | 读写属性 | 类型                              | 必填 | 描述                                                         |
  | ----------- | -------- | --------------------------------- | ---- | ------------------------------------------------------------ |
  | want        | 只读     | Want                              | 是   | 指定Want信息                                                 |
  | bundleFlags | 只读     | number                            | 是   | 0：返回默认BundleInfo<br/>1：返回包含abilityInfo的BundleInfo |
  | userId      | 只读     | number                            | 是   | 用户ID                                                       |
  | callback    | 只读     | AsyncCallback<Array<AbilityInfo>> | 是   | 回调方法                                                     |

* Want类型说明

  | 名称        | 读写属性 | 类型                 | 必填 | 描述                                                         |
  | ----------- | -------- | -------------------- | ---- | ------------------------------------------------------------ |
  | elementName | 只读     | ElementName          | 是   | 表示运行指定Ability的ElementName。                           |
  | uri         | 只读     | string               | 否   | 表示Uri描述。                                                |
  | flags       | 只读     | int                  | 否   | Ability的flag，表示处理Want的方式。                          |
  | type        | 只读     | string               | 否   | Want中的type属性是指由Want的URI所指示的资源的MIME类型。      |
  | action      | 只读     | string               | 否   | 表示动作，通常使用系统预置Action，应用也可以自定义Action。   |
  | want_param  | 只读     | {[key: string]: any} | 否   | want_param是一种支持自定义的数据结构，开发者可以通过want_param传递某些请求所需的额外信息。 |
  | entities    | 只读     | Array<string>        | 否   | 表示类别，通常使用系统预置Entity，应用也可以自定义Entity。   |

* ElementName类型说明

  | 名称        | 读写属性 | 类型   | 必填 | 描述                                                         |
  | ----------- | -------- | ------ | ---- | ------------------------------------------------------------ |
  | deviceId    | 只读     | string | 否   | 表示运行指定Ability的设备ID。                                |
  | bundleName  | 只读     | string | 是   | 表示包描述。如果在Want中同时指定了BundleName和AbilityName，则Want可以直接匹配到指定的Ability。 |
  | abilityName | 只读     | string | 是   | 表示待启动的Ability名称。如果在Want中同时指定了BundleName和AbilityName，则Want可以直接匹配到指定的Ability。 |

* 返回值

  void

* 示例

```
bundle.queryAbilityByWant(
    {
        want: {
            action: "action.system.home",
            entities: ["entity.system.home"],
            elementName: {
                deviceId: "0",
                bundleName: "com.example.myapplicationInstall",
                abilityName: "com.example.myapplication.MainAbility",
            },
        }
    }, 1, 0,
    }, OnReceiveEvent);

function OnReceiveEvent(err, data) {
    console.info("name:" + data.name);
    console.info("label:" + data.label);
    console.info("description:" + data.description);
    console.info("iconPath:" + data.iconPath);
    console.info("visible:" + data.visible);
    console.info("kind:" + data.kind);
    console.info("uri:" + data.uri);
    console.info("process:" + data.process);
    console.info("package:" + data.package);
    console.info("bundleName:" + data.bundleName);
    console.info("moduleName:" + data.moduleName);
    console.info("applicationName:" + data.applicationName);
    console.info("deviceId:" + data.deviceId);
    console.info("codePath:" + data.codePath);
    console.info("resourcePath:" + data.resourcePath);
    console.info("libPath:" + data.libPath);

    console.info('queryAbilityInfo permissions length [' + data.permissions.length + ']');
    for (var j = 0; j < data.permissions.length; j++) {
        console.info("permissions[" + j + "]:" + data.permissions[j]);
    }
    console.info('queryAbilityInfo deviceTypes length [' + data.deviceTypes.length + ']');
    for (var j = 0; j < data.deviceTypes.length; j++) {
        console.info("deviceTypes[" + j + "]:" + data.deviceTypes[j]);
    }
    console.info('queryAbilityInfo deviceCapabilities length [' + data.deviceCapabilities.length + ']');
    for (var j = 0; j < data.deviceCapabilities.length; j++) {
        console.info("deviceCapabilities[" + j + "]:" + data.deviceCapabilities[j]);
    }
    console.info("appInfo.name:" + data.applicationInfo.name);
    console.info("appInfo.bundleName:" + data.applicationInfo.bundleName);
    // ability type: 0:UNKNOWN, 1:PAGE, 2:SERVICE, 3:DATA
    console.info("type:" + data.type);
    // orientation: 0:UNSPECIFIED, 1:LANDSCAPE, 2:PORTRAIT, 3:FOLLOWRECENT,
    console.info("orientation:" + data.orientation);
    // launchMode: 0:SINGLETON, 1:SINGLETOP, 2:STANDARD
    console.info("launchMode:" + data.launchMode);

    // the enum of AbilityType
    console.info("AbilityType:" + bundle.AbilityType.UNKNOWN);
    console.info("AbilityType:" + bundle.AbilityType.PAGE);
    console.info("AbilityType:" + bundle.AbilityType.SERVICE);
    console.info("AbilityType:" + bundle.AbilityType.DATA);
    if (data.type == bundle.AbilityType.PAGE) {
        console.info("this AbilityType is PAGE");
    }
    // the enum of DisplayOrientation
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.UNSPECIFIED);
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.LANDSCAPE);
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.PORTRAIT);
    console.info("DisplayOrientation:" + bundle.DisplayOrientation.FOLLOWRECENT);
    if (data.orientation == bundle.DisplayOrientation.UNSPECIFIED) {
        console.info("this DisplayOrientation is UNSPECIFIED");
    }
    // the enum of LaunchMode
    console.info("LaunchMode:" + bundle.LaunchMode.SINGLETON);
    console.info("LaunchMode:" + bundle.LaunchMode.SINGLETOP);
    console.info("LaunchMode:" + bundle.LaunchMode.STANDARD);
    if (data.launchMode == bundle.LaunchMode.STANDARD) {
        console.info("this LaunchMode is STANDARD");
    }
}
```

#### 安装hap包

* install参数描述

  | 名称            | 读写属性 | 类型                         | 必填 | 描述                                                         |
  | --------------- | -------- | ---------------------------- | ---- | ------------------------------------------------------------ |
  | bundleFilePaths | 只读     | Array<string>                | 是   | 安装用包路径                                                 |
  | param           | 只读     | InstallParam                 | 是   | userId：用户ID<br/>installFlag：安装标识。<br/>      NORMAL：安装/卸载<br/>      REPLACE_EXISTING：更新<br/>isKeepData：卸载时是否保留运行时数据 |
  | callback        | 只读     | AsyncCallback<InstallStatus> | 是   | 回调方法                                                     |

* 返回值

  void

* InstallStatus类型说明

  | 名称          | 读写属性 | 类型            | 必填 | 描述     |
  | ------------- | -------- | --------------- | ---- | -------- |
  | InstallStatus | 只读     | IStatusReceiver | 是   | 安装结果 |

* 示例

```
bundle.getBundleInstaller().then((data) => {
    data.install(['/data/test.hap'], {
        param: {
            userId: 0,
            isKeepData: false
        }
    }, OnReceiveinstallEvent);

    function OnReceiveinstallEvent(err, data) {
        console.info("name: for begin");
        console.info("install result code:" + data.status);
        console.info("install result msg:" + data.statusMessage);
    }
})
```


#### 卸载hap包

* uninstall参数描述

  | 名称       | 读写属性 | 类型                         | 必填 | 描述                                                         |
  | ---------- | -------- | ---------------------------- | ---- | ------------------------------------------------------------ |
  | bundleName | 只读     | string                       | 是   | 卸载用包名                                                   |
  | param      | 只读     | InstallParam                 | 是   | userId：用户ID<br/>installFlag：安装标识。<br/>      NORMAL：安装/卸载<br/>      REPLACE_EXISTING：更新<br/>isKeepData：卸载时是否保留运行时数据 |
  | callback   | 只读     | AsyncCallback<InstallStatus> | 是   | 回调方法                                                     |

* 返回值

  void

* InstallStatus类型说明

  | 名称          | 读写属性 | 类型            | 必填 | 描述     |
  | ------------- | -------- | --------------- | ---- | -------- |
  | InstallStatus | 只读     | IStatusReceiver | 是   | 卸载结果 |

* 示例

```
bundle.getBundleInstaller().then((data) => {
    data.uninstall('com.example.myapplication', {
        param: {
            userId: 0,
            isKeepData: false
        }
    }, OnReceiveinstallEvent);

    function OnReceiveinstallEvent(err, data) {
        console.info("name: for begin");
        console.info("uninstall result code:" + data.status);
        console.info("uninstall result msg:" + data.statusMessage);
    }
})
```


### bm命令如下

**bm命令帮助**

| 命令    | 描述       |
| ------- | ---------- |
| bm help | bm帮助命令 |

**安装应用**

| 命令                                | 描述                       |
| ----------------------------------- | -------------------------- |
| bm install -p <bundle-file-path>    | 通过指定路径安装一个应用包 |
| bm install -r -p <bundle-file-path> | 覆盖安装一个应用包         |

```
示例如下：
bm install -p /data/app/ohosapp.hap
```

**卸载应用**

| 命令                          | 描述                     |
| ----------------------------- | ------------------------ |
| bm uninstall -n <bundle-name> | 通过指定包名卸载一个应用 |

```
示例如下：
bm uninstall -n com.ohos.app
```

**查看应用安装信息**

| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm dump -a | 列出系统已经安装的所有应用 |

## 相关仓

用户程序框架子系统

**appexecfwk_standard**

aafwk_standard

startup_appspawn

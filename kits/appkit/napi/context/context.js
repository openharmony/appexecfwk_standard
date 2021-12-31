/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

class Context {
    constructor(obj) {
        this.__context_impl__ = obj

        this.resourceManager = obj.resourceManager
        this.applicationInfo = obj.applicationInfo
        this.cacheDir = obj.cacheDir
        this.tempDir = obj.tempDir
        this.filesDir = obj.filesDir
        this.distributedFilesDir = obj.distributedFilesDir
        this.databaseDir = obj.databaseDir
        this.storageDir = obj.storageDir
        this.bundleCodeDir = obj.bundleCodeDir
    }

    createBundleContext(bundleName) {
        return this.__context_impl__.createBundleContext(bundleName)
    }
    getApplicationContext() {
        return this.__context_impl__.getApplicationContext()
    }
}

export default Context

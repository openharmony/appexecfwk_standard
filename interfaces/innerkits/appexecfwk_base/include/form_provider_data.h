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

#ifndef FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_DATA_H
#define FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_DATA_H

#include <cstddef>
#include <map>
#include <string>

#include "ashmem.h"
#include "message_parcel.h"
#include "nlohmann/json.hpp"
#include "parcel.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @class FormProviderData
 * Defines form provider data.
 */
class FormProviderData : public Parcelable {
public:
    /**
     * @brief Constructor.
     */
    FormProviderData();

    /**
     * @brief A constructor used to create a {@code FormProviderData} instance with data of
     * the {@code nlohmann::json} type specified.
     * @param jsonData Indicates the data to be carried in the new {@code FormProviderData} instance,
     * in {@code nlohmann::json} format.
     */
    FormProviderData(nlohmann::json &jsonData);

    /**
     * @brief A constructor used to create a {@code FormProviderData} instance with data of the {@code String} type 
     * specified.
     * @param jsonDataString Indicates the data to be carried in the new {@code FormProviderData} instance, in JSON 
     * string format.
     */
    FormProviderData(std::string jsonDataString);

    /**
     * @brief Destructor.
     */
    virtual ~FormProviderData(){};

    /**
     * @brief Updates form data in this {@code FormProviderData} object.
     * @param jsonData Indicates the new data to use, in {@code nlohmann::json} format.
     */
    void UpdateData(nlohmann::json &jsonData);

    /**
     * @brief Obtains the form data stored in this {@code FormProviderData} object.
     * @return Returns json string format
     */
    std::string GetDataString() const;

    /**
     * @brief Adds an image to this {@code FormProviderData} instance.
     * @param picName Indicates the name of the image to add.
     * @param data Indicates the binary data of the image content.
     */    
    void AddImageData(std::string  picName, char *data);

    /**
     * @brief Removes data of an image with the specified {@code picName} from this {@code FormProviderData} instance.
     * @param picName Indicates the name of the image to remove.
     */    
    void RemoveImageData(std::string picName);

    /**
     * @brief Obtains the add/remove state stored in this {@code FormProviderData} object.
     * @return Returns the add/remove state of shared image data.
     */    
    int32_t GetImageDataState();

    /**
     * @brief Updates imageDataState in this {@code FormProviderData} object.
     * @param imageDataState Indicates the imageDataState to update.
     */    
    void SetImageDataState(int32_t imageDataState);

    /**
     * @brief Obtains the imageDataMap stored in this {@code FormProviderData} object.
     * @return Returns the map that contains shared image data.
     */    
    std::map<std::string, std::pair<sptr<Ashmem>, int32_t>> GetImageDataMap();

    /**
     * @brief Updates imageDataMap in this {@code FormProviderData} object.
     * @param imageDataMap Indicates the imageDataMap to update.
     */    
    void SetImageDataMap(std::map<std::string, std::pair<sptr<Ashmem>, int32_t>> imageDataMap);

    /**
     * @brief Obtains the form data stored in this {@code FormProviderData} object.
     * @return Returns json data
     */
    nlohmann::json GetData() const;
    /**
     * @brief Set the form data stored from string string.
     * @param Returns string string.
     */
    void SetDataString(std::string &jsonDataString);
    /**
     * @brief Merge new data to FormProviderData.
     * @param addJsonData data to merge to FormProviderData
     */
    void MergeData(nlohmann::json &addJsonData);

    /**
     * Read this {@code FormProviderData} object from a Parcel.
     * @param parcel the parcel
     * eturn Returns {@code true} if the marshalling is successful; returns {@code false} otherwise.
     */
    bool ReadFromParcel(Parcel &parcel);
     /**
     * @brief Marshals this {@code FormProviderData} object into a {@link ohos.utils.Parcel} object.
     * @param parcel Indicates the {@code Parcel} object for marshalling.
     * @return Returns {@code true} if the marshalling is successful; returns {@code false} otherwise.
     */
    virtual bool Marshalling(Parcel &parcel) const override;
    
    /**
     * @brief Unmarshals this {@code FormProviderData} object from a {@link ohos.utils.Parcel} object.
     * @param parcel Indicates the {@code Parcel} object for unmarshalling.
     * @return Returns FormProviderData.
     */
    static FormProviderData* Unmarshalling(Parcel &parcel);

    /**
     * @brief Clear imageDataMap, rawImageBytesMap, imageDataState and jsonFormProviderData.
     */
    void ClearData();

private:
    bool WriteImageDataToParcel(Parcel &parcel, std::string picName, char *data) const;

private:
    nlohmann::json jsonFormProviderData_;
    std::map<std::string, std::pair<sptr<Ashmem>, int32_t>> imageDataMap_;
    std::map<std::string, char *> rawImageBytesMap_;
    int32_t imageDataState_ = 0;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // FOUNDATION_APPEXECFWK_OHOS_FORM_PROVIDER_DATA_H

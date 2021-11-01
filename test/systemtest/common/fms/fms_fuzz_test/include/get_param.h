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
#ifndef GET_PARAM_H
#define GET_PARAM_H
#include <memory>
#include <cstdint>
#include <string>
#include <vector>

#include "ability.h"
#include "form_info.h"
#include "form_js_info.h"
#include "pac_map.h"
#include "parcel.h"
#include "patterns_matcher.h"
#include "uri.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
bool GetBoolParam();
uint8_t GetU8Param();
unsigned int GetUIntParam();
uint16_t GetU16Param();
uint32_t GetU32Param();
uint64_t GetU64Param();
int8_t GetS8Param();
int16_t GetS16Param();
int32_t GetS32Param();
int64_t GetS64Param();
char32_t GetChar32Param();

short GetShortParam();
long GetLongParam();
int GetIntParam();
double GetDoubleParam();
float GetFloatParam();
char GetCharParam();
char *GetCharArryParam();
std::string GetStringParam();
std::map<int64_t, int32_t> GetS64S32MapParam();
std::vector<bool> GetBoolVectorParam();
std::vector<short> GetShortVectorParam();
std::vector<long> GetLongVectorParam();
std::vector<int> GetIntVectorParam();
std::vector<float> GetFloatVectorParam();
std::vector<double> GetDoubleVectorParam();
std::vector<char> GetCharVectorParam();
std::vector<char32_t> GetChar32VectorParam();
std::vector<std::string> GetStringVectorParam();
template<class T>
std::vector<T> GetUnsignVectorParam();
std::vector<int8_t> GetS8VectorParam();
std::vector<int16_t> GetS16VectorParam();
std::vector<int32_t> GetS32VectorParam();
std::vector<int64_t> GetS64VectorParam();
std::shared_ptr<OHOS::AppExecFwk::Ability> GetParamAbility();
OHOS::AppExecFwk::Want GetParamWant();
OHOS::AppExecFwk::FormInfo GetParamFormInfo();
std::vector<OHOS::AppExecFwk::FormInfo> GetmFormInfoVectorParam();
OHOS::AppExecFwk::FormJsInfo GetParamFormJsInfo();
std::shared_ptr<OHOS::AppExecFwk::Ability::FormCallback> GetParamFormCallback();
sptr<OHOS::AppExecFwk::Ability> GetParamSptrAbility();
OHOS::AppExecFwk::FormProviderData GetParamFormProviderData();

class GetParam : public Ability {
public:
    GetParam();
    ~GetParam();

    bool ReleaseForm(const int64_t formId);

    /**
     * @brief Releases an obtained form by its ID.
     *
     * <p>After this method is called, the form won't be available for use by the application, if isReleaseCache is
     * false, this method is same as {@link #releaseForm(int)}, otherwise the Form Manager Service still store this
     * form in the cache.</p>
     * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
     *
     * @param formId Indicates the form ID.
     * @param isReleaseCache Indicates whether to clear cache in service.
     * @return Returns {@code true} if the form is successfully released; returns {@code false} otherwise.
     *
     * <ul>
     * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
     * <li>The specified form has not been added by the application.</li>
     * <li>An error occurred when connecting to the Form Manager Service.</li>
     * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
     * <li>The form has been obtained by another application and cannot be released by the current application.</li>
     * <li>The form is being restored.</li>
     * </ul>
     */
    bool ReleaseForm(const int64_t formId, const bool isReleaseCache);

    /**
     * @brief Deletes an obtained form by its ID.
     *
     * <p>After this method is called, the form won't be available for use by the application and the Form Manager
     * Service no longer keeps the cache information about the form.</p>
     * <p><b>Permission: </b>{@link ohos.security.SystemPermission#REQUIRE_FORM}</p>
     *
     * @param formId Indicates the form ID.
     * @return Returns {@code true} if the form is successfully deleted; returns {@code false} otherwise.
     *
     * <ul>
     * <li>The passed {@code formId} is invalid. Its value must be larger than 0.</li>
     * <li>The specified form has not been added by the application.</li>
     * <li>An error occurred when connecting to the Form Manager Service.</li>
     * <li>The application is not granted with the {@link ohos.security.SystemPermission#REQUIRE_FORM} permission.</li>
     * <li>The form has been obtained by another application and cannot be deleted by the current application.</li>
     * <li>The form is being restored.</li>
     * </ul>
     */
    bool DeleteForm(const int64_t formId);

    /**
     * @brief The form callback.
     */
    class FormCallback {
    public:

        /**
         * @brief Called to notify the application that the {@code FormJsInfo} instance has been obtained after
         * the application called the asynchronous method {@link Ability#acquireForm(Want, FormCallback)}.
         * The application must present the form information on a specific page in this callback.
         *
         * @param result Specifies whether the asynchronous form acquisition process is successful.
         *               {@link FormCallback#OHOS_FORM_ACQUIRE_SUCCESS} indicates that the form
         *               is successfully obtained, and other values indicate that the process fails.
         * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
         */
        virtual void OnAcquired(const int32_t result, const FormJsInfo &formJsInfo) const = 0;

        /**
         * @brief Called to notify the application that the {@code FormJsInfo} instance has been obtained after
         * the application called the asynchronous method {@link Ability#acquireForm(Want, FormCallback)}.
         * The application must present the form information on a specific page in this callback.
         *
         * @param result Specifies whether the asynchronous form acquisition process is successful.
         *               {@link FormCallback#OHOS_FORM_UPDATE_SUCCESS} indicates that the form is
         *               successfully obtained, and other values indicate that the process fails.
         * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
         */
        virtual void OnUpdate(const int32_t result, const FormJsInfo &formJsInfo) const = 0;

        /**
         * @brief Called to notify the application that the {@code Form} provider has been uninstalled and the
         * corresponding
         * {@code Form} instance is no longer available.
         *
         * @param formId Indicates the ID of the {@code Form} instance provided by the uninstalled form provider.
         */
        virtual void OnFormUninstall(const int64_t formId) const = 0;
    };

    /**
     * @brief Obtains a specified form that matches the application bundle name, module name, form name, and
     * other related information specified in the passed {@code Want}.
     *
     * <p>This method is asynchronous. After the {@link FormJsInfo} instance is obtained.
     *
     * @param formId Indicates the form ID.
     * @param want Indicates the detailed information about the form to be obtained, including the bundle name,
     *        module name, ability name, form name, form id, tempForm flag, form dimension, and form customize data.
     * @param callback Indicates the callback to be invoked whenever the {@link FormJsInfo} instance is obtained.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool AcquireForm(const int64_t formId, const Want &want, const std::shared_ptr<FormCallback> callback);

    /**
     * @brief Updates the content of a specified JS form.
     *
     * <p>This method is called by a form provider to update JS form data as needed.
     *
     * @param formId Indicates the form ID.
     * @param formProviderData The data used to update the JS form displayed on the client.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool UpdateForm(const int64_t formId, const FormProviderData &formProviderData);

    /**
     * @brief Cast temp form with formId.
     *
     * @param formId Indicates the form's ID.
     *
     * @return Returns {@code true} if the form is successfully casted; returns {@code false} otherwise.
     */
    bool CastTempForm(const int64_t formId);

    /**
     * @brief Sends a notification to the form framework to make the specified forms visible.
     *
     * <p>After this method is successfully called, {@link Ability#OnVisibilityChanged(std::map<int64_t, int>)}
     * will be called to notify the form provider of the form visibility change event.</p>
     *
     * @param formIds Indicates the IDs of the forms to be made visible.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool NotifyVisibleForms(const std::vector<int64_t> &formIds);

    /**
     * @brief Sends a notification to the form framework to make the specified forms invisible.
     *
     * <p>After this method is successfully called, {@link Ability#OnVisibilityChanged(std::map<int64_t, int>)}
     * will be called to notify the form provider of the form visibility change event.</p>
     *
     * @param formIds Indicates the IDs of the forms to be made invisible.
     * @return Returns {@code true} if the request is successfully initiated; returns {@code false} otherwise.
     */
    bool NotifyInvisibleForms(const std::vector<int64_t> &formIds);

    /**
     * @brief Set form next refresh time.
     *
     * <p>This method is called by a form provider to set refresh time.
     *
     * @param formId Indicates the ID of the form to set refresh time.
     * @param nextTime Indicates the next time gap now in seconds, can not be litter than 300 seconds.
     * @return Returns {@code true} if seting succeed; returns {@code false} otherwise.
     */

    bool SetFormNextRefreshTime(const int64_t formId, const int64_t nextTime);

    /**
     * @brief Update form.
     *
     * @param formJsInfo Indicates the obtained {@code FormJsInfo} instance.
     */
    void ProcessFormUpdate(const FormJsInfo &formJsInfo) override;

    /**
     * @brief Uninstall form.
     *
     * @param formId Indicates the ID of the form to uninstall.
     */
    void ProcessFormUninstall(const int64_t formId) override;

    /**
     * @brief Called to reacquire form and update the form host after the death callback is received.
     *
     */
    void OnDeathReceived() override;

    /**
     * @brief Called to return a FormProviderInfo object.
     *
     * <p>You must override this method if your ability will serve as a form provider to provide a form for clients.
     * The default implementation returns nullptr. </p>
     *
     * @param want   Indicates the detailed information for creating a FormProviderInfo.
     *               The Want object must include the form ID, form name of the form,
     *               which can be obtained from Ability#PARAM_FORM_IDENTITY_KEY,
     *               Ability#PARAM_FORM_NAME_KEY, and Ability#PARAM_FORM_DIMENSION_KEY,
     *               respectively. Such form information must be managed as persistent data for further form
     *               acquisition, update, and deletion.
     *
     * @return Returns the created FormProviderInfo object.
     */
    virtual FormProviderInfo OnCreate(const Want &want) override;

    /**
     * @brief Called to notify the form provider that a specified form has been deleted. Override this method if
     * you want your application, as the form provider, to be notified of form deletion.
     *
     * @param formId Indicates the ID of the deleted form.
     * @return None.
     */
    virtual void OnDelete(const int64_t formId) override;

    /**
     * @brief Called when the form provider is notified that a temporary form is successfully converted to
     * a normal form.
     *
     * @param formId Indicates the ID of the form.
     * @return None.
     */
    virtual void OnCastTemptoNormal(const int64_t formId) override;

    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @return none.
     */
    virtual void OnUpdate(const int64_t formId) override;

    /**
     * @brief Called when the form provider receives form events from the fms.
     *
     * @param formEventsMap Indicates the form events occurred. The key in the Map object indicates the form ID,
     *                      and the value indicates the event type, which can be either FORM_VISIBLE
     *                      or FORM_INVISIBLE. FORM_VISIBLE means that the form becomes visible,
     *                      and FORM_INVISIBLE means that the form becomes invisible.
     * @return none.
     */
    virtual void OnVisibilityChanged(const std::map<int64_t, int32_t> &formEventsMap) override;
    /**
     * @brief Called to notify the form provider to update a specified form.
     *
     * @param formId Indicates the ID of the form to update.
     * @param message Form event message.
     */
    virtual void OnTriggerEvent(const int64_t formId, const std::string &message) override;
    /**
     * @brief Requests for form data update.
     *
     * This method must be called when the application has detected that a system setting item (such as the language,
     * resolution, or screen orientation) being listened for has changed. Upon receiving the update request, the form
     * provider automatically updates the form data (if there is any update) through the form framework, with the update
     * process being unperceivable by the application.
     *
     * @param formId Indicates the ID of the form to update.
     * @return Returns true if the update request is successfully initiated, returns false otherwise.
     */
    bool RequestForm(const int64_t formId);

    /**
     * @brief Requests for form data update, by passing a set of parameters (using Want) to the form provider.
     *
     * This method must be called when the application has detected that a system setting item (such as the language,
     * resolution, or screen orientation) being listened for has changed. Upon receiving the update request, the form
     * provider automatically updates the form data (if there is any update) through the form framework, with the update
     * process being unperceivable by the application.
     *
     * @param formId Indicates the ID of the form to update.
     * @param want Indicates a set of parameters to be transparently passed to the form provider.
     * @return Returns true if the update request is successfully initiated, returns false otherwise.
     */
    bool RequestForm(const int64_t formId, const Want &want);
    /**
     * @brief Enable form update.
     *
     * @param formIds formIds of hostclient.
     */
    bool EnableUpdateForm(const std::vector<int64_t> &formIds);

    /**
     * @brief Disable form update.
     *
     * @param formIds formIds of hostclient.
     */
    bool DisableUpdateForm(const std::vector<int64_t> &formIds);

    /**
     * @brief Check form manager service ready.
     *
     * @return Returns true if form manager service ready; returns false otherwise.
     */
    bool CheckFMSReady();

    /**
     * @brief Get All FormsInfo.
     *
     * @param formInfos Returns the forms' information of all forms provided.
     * @return Returns true if the request is successfully initiated; returns false otherwise.
     */
    bool GetAllFormsInfo(std::vector<FormInfo> &formInfos);

    /**
     * @brief Get forms info by application name.
     *
     * @param bundleName Application name.
     * @param formInfos Returns the forms' information of the specify application name.
     * @return Returns true if the request is successfully initiated; returns false otherwise.
     */
    bool GetFormsInfoByApp(std::string &bundleName, std::vector<FormInfo> &formInfos);

    /**
     * @brief Get forms info by application name and module name.
     *
     * @param bundleName Application name.
     * @param moduleName Module name of hap.
     * @param formInfos Returns the forms' information of the specify application name and module name.
     * @return Returns true if the request is successfully initiated; returns false otherwise.
     */
    bool GetFormsInfoByModule(std::string &bundleName, std::string &moduleName, std::vector<FormInfo> &formInfos);
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
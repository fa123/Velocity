#ifndef XCONTENTDEVICEPROFILE_H
#define XCONTENTDEVICEPROFILE_H

#include <iostream>
#include <vector>

#include "Stfs/StfsPackage.h"
#include "XContentDeviceItem.h"
#include "XContentDeviceTitle.h"

#include "XboxInternals_global.h"

class XBOXINTERNALSSHARED_EXPORT XContentDeviceProfile : public XContentDeviceItem
{
public:
    XContentDeviceProfile(std::string pathOnDevice, std::string rawName, StfsPackage *profile);

    std::vector<XContentDeviceTitle> titles;

    BYTE* GetProfileID();
};

#endif // XCONTENTDEVICEPROFILE_H


#include "Zeon/Public/Utilities/ZeonWorldUtils.h"

TUniquePtr<FZeonWorldUtils> FZeonWorldUtils::Instance;
FDelegateHandle FZeonWorldUtils::PostWorldInitDelegateHandle;
FZeonWorldUtils::FOnWorldBeginPlay FZeonWorldUtils::OnWorldBeginPlay;
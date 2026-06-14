#include "TrueSyncEndfieldShadingShaders.h"

IMPLEMENT_GLOBAL_SHADER(
	FTrueSyncEndfieldCompositePS,
	"/Plugin/TrueSyncEndfieldShading/Private/TrueSyncEndfieldComposite.usf",
	"MainPS",
	SF_Pixel);

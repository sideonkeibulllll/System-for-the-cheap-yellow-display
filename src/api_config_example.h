#ifndef API_CONFIG_EXAMPLE_H
#define API_CONFIG_EXAMPLE_H

#include <pgmspace.h>

#define AI_MODEL_NAME_MAX_LEN   16
#define AI_MODEL_ENDPOINT_MAX_LEN 64
#define AI_MODEL_KEY_MAX_LEN    64
#define AI_MODEL_ID_MAX_LEN     32

typedef struct {
    char name[AI_MODEL_NAME_MAX_LEN];
    char endpoint[AI_MODEL_ENDPOINT_MAX_LEN];
    char apiKey[AI_MODEL_KEY_MAX_LEN];
    char model[AI_MODEL_ID_MAX_LEN];
} ai_model_config_t;

static const ai_model_config_t AI_MODELS[] = {
    {"DeepSeek", "https://api.deepseek.com/chat/completions", "YOUR_DEEPSEEK_API_KEY", "deepseek-chat"},
    {"GLM-5", "https://open.bigmodel.cn/api/paas/v4/chat/completions", "YOUR_GLM_API_KEY", "glm-5"},
    {"DeepSeek-R1", "https://api.siliconflow.cn/v1/chat/completions", "YOUR_SILICONFLOW_API_KEY", "deepseek-ai/DeepSeek-R1"}
};

#define AI_MODEL_COUNT (sizeof(AI_MODELS) / sizeof(AI_MODELS[0]))

#endif

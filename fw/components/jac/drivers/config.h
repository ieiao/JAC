#ifndef __CONFIG_H__
#define __CONFIG_H__

#define KEY_CONFIGURED      "k1"
#define KEY_PS_BEGIN        "k2"
#define KEY_PS_END          "k3"

int config_init(void);
void config_deinit(void);
int config_erase(void);
int __config_load(struct jac_info *info);
int config_load(struct jac_info *info);
int config_set(const char *k, uint8_t v);

#endif

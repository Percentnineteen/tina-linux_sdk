# components
snd_soc_sunxi_component_rxsync-objs		+= snd_sunxi_rxsync.o
obj-$(CONFIG_SND_SOC_SUNXI_RXSYNC)		+= snd_soc_sunxi_component_rxsync.o

ifneq (, $(filter y m, $(CONFIG_SND_SOC_SUNXI_JACK_CODEC)))
snd_soc_sunxi_component_jack-objs		+= snd_sunxi_jack_codec.o
else ifneq (, $(filter y m, $(CONFIG_SND_SOC_SUNXI_JACK_EXTCON)))
snd_soc_sunxi_component_jack-objs		+= snd_sunxi_jack_extcon.o
endif
ifeq (, $(filter y m, $(CONFIG_SND_SOC_SUNXI_JACK_NONE)))
obj-$(CONFIG_SND_SOC_SUNXI_JACK)		+= snd_soc_sunxi_component_jack.o
endif

# common -> platform of dma
snd_soc_sunxi_pcm-objs				+= snd_sunxi_pcm.o
obj-$(CONFIG_SND_SOC_SUNXI_PCM)			+= snd_soc_sunxi_pcm.o

# common -> platform of hdmi audio (use for ahub | daudio)
snd_soc_sunxi_pcm_hdmi-objs			+= snd_sunxi_hdmi.o
snd_soc_sunxi_pcm_hdmi-objs			+= snd_sunxi_pcm_hdmi.o
obj-$(CONFIG_SND_SOC_SUNXI_PCM_HDMI)		+= snd_soc_sunxi_pcm_hdmi.o

# codec -> internal codec
ifneq (, $(filter y m, $(CONFIG_SND_SOC_SUNXI_SUN50IW9_CODEC)))
snd_soc_sunxi_internal_codec-objs		+= snd_sun50iw9_codec.o
endif
snd_soc_sunxi_internal_codec-objs		+= snd_sunxi_common.o
obj-$(CONFIG_SND_SOC_SUNXI_INTERNALCODEC)	+= snd_soc_sunxi_internal_codec.o

# codec -> dummy codec
snd_soc_sunxi_dummy_codec-objs			+= snd_sunxi_dummy_codec.o
obj-$(CONFIG_SND_SOC_SUNXI_DUMMYCODEC)		+= snd_soc_sunxi_dummy_codec.o

# platform -> analog audio
snd_soc_sunxi_aaudio-objs			+= snd_sunxi_aaudio.o
obj-$(CONFIG_SND_SOC_SUNXI_AAUDIO)		+= snd_soc_sunxi_aaudio.o

# platform -> spdif
ifneq (, $(filter y m, $(CONFIG_SND_SOC_SUNXI_SPDIF_RXIEC61937)))
snd_soc_sunxi_spdif-objs			+= snd_sunxi_spdif_rx61937.o
endif
snd_soc_sunxi_spdif-objs			+= snd_sunxi_spdif.o
obj-$(CONFIG_SND_SOC_SUNXI_SPDIF)		+= snd_soc_sunxi_spdif.o

# platform -> dmic
snd_soc_sunxi_dmic-objs				+= snd_sunxi_dmic.o
obj-$(CONFIG_SND_SOC_SUNXI_DMIC)		+= snd_soc_sunxi_dmic.o

# platform -> daudio
snd_soc_sunxi_daudio-objs			+= snd_sunxi_daudio.o
obj-$(CONFIG_SND_SOC_SUNXI_DAUDIO)		+= snd_soc_sunxi_daudio.o

# platform -> ahub
snd_soc_sunxi_ahub_dam-objs			+= snd_sunxi_ahub_dam.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB_DAM)		+= snd_soc_sunxi_ahub_dam.o

snd_soc_sunxi_ahub-objs				+= snd_sunxi_ahub.o
obj-$(CONFIG_SND_SOC_SUNXI_AHUB)		+= snd_soc_sunxi_ahub.o

# common -> machine (note: Finally compile, save system startup time)
snd_soc_sunxi_machine-objs			+= snd_sunxi_mach.o
snd_soc_sunxi_machine-objs			+= snd_sunxi_mach_utils.o
obj-$(CONFIG_SND_SOC_SUNXI_MACH)		+= snd_soc_sunxi_machine.o

#import <sound/sound.h>
#import <soundkit/soundkit.h>

id soundout;

void printCurrentParameters()
{
	int isMuted = [soundout intValueForParameter: NX_SoundDeviceMuteLineOut];
	int att = [soundout intValueForParameter: NX_SoundDeviceOutputAttenuationStereo];
	int attL = [soundout intValueForParameter: NX_SoundDeviceOutputAttenuationLeft];
	int attR = [soundout intValueForParameter: NX_SoundDeviceOutputAttenuationRight];

	printf("muted=%d\n", isMuted);
	printf("att stereo=%d\n", att);
	printf("att left=%d\n", attL);
	printf("att right=%d\n", attR);
}

void main(int argc, char *argv[]) {
	
	char action;
	int value;
	soundout = [NXSoundOut new];
	
	for(;;) {
		action = 0;
		printCurrentParameters();
		printf("input action, l=set left, r=set right, s=set stereo, e=exit\n");
		scanf("%c", &action);
		switch (action) {
			case 'l':
			printf("input attenuation value: ");
			scanf("%d", &value);
			printf("v=%d\n", value);
			[soundout setParameter: NX_SoundDeviceOutputAttenuationLeft toInt: value];
			break;
			case 'r':
			printf("input attenuation value: ");
			scanf("%d", &value);
			[soundout setParameter: NX_SoundDeviceOutputAttenuationRight toInt: value];
			break;
			case 's':
			printf("input attenuation value: ");
			scanf("%d", &value);
			[soundout setParameter: NX_SoundDeviceOutputAttenuationStereo toInt: value];
			break;
			case 'e':
			exit(0);
			default:
			break;
		}
	}



	exit(0);
}
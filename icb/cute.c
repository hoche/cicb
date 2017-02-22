/* $Id: cute.c,v 1.9 2003/07/07 01:42:15 felixlee Exp $ */

/* cute sayings when a user hits RETURN with no text */

#include "icb.h"

/* List of chalkboard quotes comes from www.snpp.com, up to episode CABF12.
   These are only the ones written by Bart. */

static char *prompt[] = {
	"\"Bagman\" is not a legitimate career choice", 
	"\"Bart Bucks\" are not legal tender", 
	"\"Bewitched\" does not promote Satanism", 
	"\"butt.butt\" is not my E-mail address", 
	"\"Non-Flammable\" is not a challenge", 
	"\"The President did it\" is not an excuse", 
	"A belch is not an oral report", 
	"A burp is not an answer", 
	"A fire drill does not demand a fire", 
	"A trained ape could not teach gym", 
	"Adding \"just kidding\" doesn't make it okay to insult the Principal", 
	"All work and no play makes Bart a dull boy", 
	"Beans are neither fruit nor musical", 
	"Class clown is not a paid position", 
	"Coffee is not for kids", 
	"Cursive writing does not mean what I think it does", 
	"Dodgeball stops at the gym door", 
	"Everyone is tired of that Richard Gere story", 
	"Five days is not too long to wait for a gun", 
	"Fridays are not \"pants optional\"", 
	"Funny noises are not funny", 
	"Garlic gum is not funny", 
	"Goldfish don't bounce", 
	"Grammar is not a time of waste", 
	"High explosives and school don't mix", 
	"Hillbillies are people too", 
	"I am not a 32 year old woman", 
	"I am not a dentist", 
	"I am not a lean mean spitting machine", 
	"I am not a licensed hairstylist", 
	"I am not authorized to fire substitute teachers", 
	"I am not certified to remove asbestos", 
	"I am not delightfully saucy", 
	"I am not here on a fartball scholarship", 
	"I am not licensed to do anything", 
	"I am not my long-lost twin", 
	"I am not the acting President",
	"I am not the last Don", 
	"I am not the new Dalai Lama", 
	"I am not the reincarnation of Sammy Davis Jr.", 
	"I am so very tired", 
	"I can't see dead people", 
	"I cannot absolve sins", 
	"I cannot hire a substitute student", 
	"I did not invent Irish dancing", 
	"I did not learn everything I need to know in kindergarten", 
	"I did not see Elvis", 
	"I did not win the Nobel Fart Prize", 
	"I do not have diplomatic immunity", 
	"I do not have power of attorney over first graders", 
	"I have neither been there nor done that", 
	"I no longer want my MTV", 
	"I saw nothing unusual in the teacher's lounge", 
	"I was not the inspiration for \"Kramer\"", 
	"I was not the sixth Beatle",
	"I was not told to do this", 
	"I was not touched \"there\" by an angel", 
	"I will finish what I sta", 
	"I will never win an Emmy", 
	"I will not \"let the dogs out\"",
	"I will not aim for the head", 
	"I will not barf unless I'm sick", 
	"I will not belch the National Anthem", 
	"I will not bribe Principal Skinner", 
	"I will not bring sheep to class", 
	"I will not burp in class", 
	"I will not bury the new kid", 
	"I will not buy a Presidential pardon",
	"I will not call my teacher \"Hot Cakes\"", 
	"I will not call the principal \"spud head\"", 
	"I will not carve gods", 
	"I will not celebrate meaningless milestones", 
	"I will not charge admission to the bathroom", 
	"I will not complain about the solution when I hear it", 
	"I will not conduct my own fire drills", 
	"I will not create art from dung", 
	"I will not cut corners", 
	"I will not dance on anyone's grave", 
	"I will not defame New Orleans", 
	"I will not demand what I'm worth", 
	"I will not dissect things unless instructed", 
	"I will not do anything bad ever again", 
	"I will not do that thing with my tongue", 
	"I will not do the Dirty Bird", 
	"I will not draw naked ladies in class", 
	"I will not drive the principal's car", 
	"I will not eat things for money", 
	"I will not encourage others to fly", 
	"I will not expose the ignorance of the faculty", 
	"I will not fake my way through life", 
	"I will not fake rabies", 
	"I will not fake seizures", 
	"I will not file frivolous lawsuits", 
	"I will not get very far with this attitude", 
	"I will not go near the kindergarten turtle", 
	"I will not grease the monkey bars", 
	"I will not hang donuts on my person", 
	"I will not hide behind the Fifth Amendment", 
	"I will not hide the teacher's medication",
	"I will not hide the teacher's Prozac", 
	"I will not instigate revolution", 
	"I will not lie in front of the school bus with ketchup on my face", 
	"I will not make flatuent noises in class", 
	"I will not mess with the opening credits", 
	"I will not mock Mrs. Dumbface", 
	"I will not obey the voices in my head",
	"I will not plant subiminAl messaGores",
	"I will not pledge allegiance to Bart", 
	"I will not prescribe medication", 
	"I will not publish the Principal's credit report",
	"I will not re-transmit without the express permission of Major League Baseball", 
	"I will not say \"Springfield\" just to get applause", 
	"I will not scream for ice cream", 
	"I will not sell land in Florida", 
	"I will not sell miracle cures", 
	"I will not sell my kidney on eBay", 
	"I will not sell school property", 
	"I will not send lard through the mail", 
	"I will not show off", 
	"I will not skateboard in the halls", 
	"I will not sleep through my education", 
	"I will not snap bras", 
	"I will not spank others", 
	"I will not spin the turtle", 
	"I will not squeak chalk", 
	"I will not strut around like I own the place", 
	"I will not surprise the incontinent",
	"I will not teach others to fly", 
	"I will not tease Fatty", 
	"I will not torment the emotionally frail", 
	"I will not trade pants with others", 
	"I will not use abbrev.", 
	"I will not waste chalk", 
	"I will not whittle hall passes out of soap", 
	"I will not Xerox my butt", 
	"I will not yell \"Fire\" in a crowded classroom", 
	"I will not yell \"She's Dead\" during roll call", 
	"I will only do this once a year", 
	"I will only provide a urine sample when asked",
	"I will remember to take my medication", 
	"I will return the seeing-eye dog", 
	"I will stop phoning it in", 
	"I will stop talking about the twelve inch pianist", 
	"I won't not use no double negatives", 
	"Indian burns are not our cultural heritage", 
	"It does not suck to be you", 
	"It's potato, not potatoe", 
	"Loose teeth don't need my help", 
	"Mud is not one of the 4 food groups", 
	"My butt does not deserve a website", 
	"My homework was not stolen by a one-armed man", 
	"My mom is not dating Jerry Seinfeld", 
	"My name is not Dr. Death", 
	"My suspension was not \"mutual\"", 
	"Nerve gas is not a toy", 
	"Network TV is not dead",
	"Next time it could be me on the scaffolding", 
	"No one cares what my definition of \"is\" is", 
	"No one is interested in my underpants", 
	"No one wants to hear about my sciatica", 
	"No one wants to hear from my armpits", 
	"Nobody likes sunburn slappers", 
	"Organ transplants are best left to the professionals", 
	"Pain is not the cleanser", 
	"Pork is not a verb", 
	"Ralph won't \"morph\" if you squeeze him hard enough", 
	"Rudolph's red nose is not alcohol-related", 
	"Science class should not end in tragedy",
	"Sherri does not \"got back\"", 
	"Shooting paintballs is not an art form", 
	"Silly String is not a nasal spray", 
	"Spitwads are not free speech", 
	"Substitute teachers are not scabs", 
	"Tar is not a plaything", 
	"Teacher is not a leper", 
	"The boys room is not a water park", 
	"The cafeteria deep fryer is not a toy", 
	"The Christmas Pageant does not stink", 
	"The First Amendment does not cover burping", 
	"The Good Humor man can only be pushed so far", 
	"The hamster did not have \"a full life\"",
	"The nurse is not dealing",
	"The Pledge of Allegiance does not end with Hail Satan", 
	"The principal's toupee is not a Frisbee", 
	"The truth is not out there", 
	"There are plenty of businesses like show business", 
	"There was no Roman god named \"Farticus\"", 
	"They are laughing at me, not with me", 
	"This is not a clue...or is it?", 
	"This punishment is not boring and pointless", 
	"Underwear should be worn on the inside", 
	"Wedgies are unhealthy for children and other living things", 
	NULL
};


/* Return an integer in [0, n) */
static int randint (int n)
{
#ifdef HAVE_LRAND48
	return lrand48 () % n;
#else
	return random () % n;
#endif
}


/* Shuffle the prompt[] array */

static void
shuffle (void)
{
	int k;

#ifdef HAVE_LRAND48
	srand48 (time (NULL));
#else
	srandom (time (NULL));
#endif

	for (k = 1; prompt[k] != NULL; ++k) {
		int j = randint (k + 1);
		char* tmp = prompt[j];
		prompt[j] = prompt[k];
		prompt[k] = tmp;
	}
}


static int next_prompt = 0;


void
saysomething()
{
	if (next_prompt == 0) {
		shuffle ();
	} else if (prompt[next_prompt] == NULL) {
		next_prompt = 0;
	}

	putl (prompt[next_prompt], PL_SCR);

	++next_prompt;
}

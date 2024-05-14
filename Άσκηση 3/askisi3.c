#include <avr/io.h>
#include <avr/interrupt.h>

int PER_high = 40; // Προσομοιώνουμε Τb = 2 ms
int CMP_high = 24; // Προσομοιώνουμε Db = 60% * Tb
int PER_low = 20; // Προσομοιώνουμε Τl = 1 ms
int CMP_low = 10 ; // Προσομοιώνουμε Dl = 50% * Tl
int PER_low2 = 10 ; // Προσομοιώνουμε Τl = 0,5 ms  
int CMP_low2 = 5; // Προσομοιώνουμε Dl = 50% * Tl

int high_edges_count = 0; // Μετρητής για την αποθήκευση του πλήθους των rising edges για το παλμό της βάσης
int low_edges_count = 0; // Μετρητής για την αποθήκευση του πλήθους των rising edges για το παλμό των λεπίδων
int x=0;
int press_button = 0; // Μετρητής για την αποθήκευση των φορών που πατιέται το κουμπί

void setup_TCA0_split(int PER_low, int CMP_low){
	TCA0.SPLIT.CTRLD = TCA_SPLIT_SPLITM_bm; // Ενεργοποιούμε το Split Mode για τον TCA
	TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1024_gc;	// Χρήση παράγοντα διαίρεσης 1024 
	TCA0.SPLIT.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // Ενεργοποίηση δημιουργίας κυματομορφών
	
	TCA0.SPLIT.HPER = PER_high; // Ορίζουμε τη μέγιστη τιμή TOP μέχρι την οποία θα μετρήσει ο παλμός της βάσης
	TCA0.SPLIT.HCMP0 = CMP_high; // Ορίζουμε το κύκλο λειτουργίας του παλμού της βάσης

	TCA0.SPLIT.LPER = PER_low; // Ορίζουμε τη μέγιστη τιμή TOP μέχρι την οποία θα μετρήσει ο παλμός των λεπίδων
	TCA0.SPLIT.LCMP0 = CMP_low; // Ορίζουμε το κύκλο λειτουργίας του παλμού των λεπίδων

	TCA0.SPLIT.CTRLB = TCA_SPLIT_HCMP0EN_bm | TCA_SPLIT_LCMP0EN_bm; // Eνεργοποιούμε τη διακοπή που προκαλεί το HCMP0 και το LCMP0
	TCA0.SPLIT.INTCTRL |= TCA_SPLIT_LUNF_bm;								// Eνεργοποιούμε τη διακοπή που προκαλεί το LUNF κατά την υπερχείλιση του χρονιστή
	TCA0.SPLIT.CTRLA |= TCA_SPLIT_CLKSEL_DIV1024_gc | TCA_SPLIT_ENABLE_bm;  // Ενεργοποιούμε τον TCA0 και ξεκινάμε το ρολόι
}

int main(void)
{
	PORTD.DIR = PIN0_bm | PIN1_bm; // Ορίζουμε τα pins 0, 1 ως εξόδους
	PORTD.OUT = PIN0_bm | PIN1_bm; // Αρχικά και τα δύο pin είναι σβηστά
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Ορίζουμε το pin 5 ως είσοδο
	sei(); // Ξεκινάμε να δεχόμαστε σήματα διακοπής
	
    while (x==0)
    {	   
		;
    }
	cli(); // Σταματάμε να δεχόμαστε σήματα διακοπής
}

// Διακοπή για το χειρισμό του πατήματος του κουμπιού
ISR(PORTF_PORT_vect){				
	int y = PORTF.INTFLAGS; // Καθαρίζουμε τις σημαίες διακοπής
	PORTF.INTFLAGS=y;
	press_button++ ; // Αυξάνουμε τις φορές που πατήθηκε το κουμπί κατα 1
	
	if(press_button==1){
		setup_TCA0_split(PER_low, CMP_low); // Αν το κουμπί πατηθεί 1η φορά αρχικοποιούμε τον TCA0 με τις αντίστοιχες τιμές
	}
	else if(press_button==2){
	   PORTD.OUT = PIN0_bm | PIN1_bm; // Σβήνουμε και τα δύο pins
	   high_edges_count = 0; // Αρχικοποιούμε και τους δύο μετρητές πάλι με 0 για να κάνουμε restart
	   low_edges_count = 0;
	   TCA0.SINGLE.CNT = 0; // Kαθαρίζουμε τον TCA0
	   setup_TCA0_split(PER_low2, CMP_low2); // Αν το κουμπί πατηθεί 2η φορά αρχικοποιούμε τον TCA0 με τη καινούρια τιμή του PER_low
	}
	else{
		TCA0.SPLIT.CTRLA = 0b00000000; // Αν το κουμπί πατηθεί 3η φορά απενεργοποιούμε τον TCA0
		x=1;					
		PORTD.OUT |= PIN0_bm | PIN1_bm; // Σβήνουμε και τα δύο pins
	}
}

// Διακοπή για το χειρισμό ανερχόμενης ακμής από τη κυματομορφή των low bits του TCA0 (λεπίδες)
ISR(TCA0_LUNF_vect) {	
	low_edges_count++; // Αυξάνουμε το πλήθος των ανερχόμενων ακμών των λεπίδων κατά 1									 
	if ((TCA0.SPLIT.INTFLAGS & 0b00000010)== 0b00000010) // Ελέγχουμε αν έχουμε ανερχόμενη ακμή από τη κυματομορφή των high bits του TCA0 (βάση)
	{		
		high_edges_count++; // Αυξάνουμε το πλήθος των ανερχόμενων ακμών των λεπίδων κατά 1
		if(high_edges_count%2==1){ // Ελέγχουμε αν έχουμε περιττό πλήθος ανερχόμενων ακμών των λεπίδων
				PORTD.OUTCLR = PIN0_bm | PIN1_bm; // Ανάβουμε και τα δύο pins
		 }
		else{
			PORTD.OUT |=PIN1_bm; // Σβήνουμε το pin 1
			PORTD.OUTCLR = PIN0_bm; // Ανάβουμε το pin 0	
		}
	}
	else{
		if(low_edges_count%2==1){ // Ελέγχουμε αν έχουμε περιττό πλήθος ανερχόμενων ακμών της βάσης
			PORTD.OUTCLR = PIN0_bm ; // Ανάβουμε το pin 0	
		}
		else{
			PORTD.OUT |= PIN0_bm; // Σβήνουμε το pin 0
		}
		
	}
		
	int intflags = TCA0.SPLIT.INTFLAGS; // masking
	TCA0.SPLIT.INTFLAGS = intflags;
}

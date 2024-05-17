#include <avr/io.h>
#include <avr/interrupt.h>

// Ορίζουμε τις χρονικές σταθερές
#define T1 30 
#define T2_T3 20 // Τ2=Τ3

int x=0; //logic flag
int button_pressed = 0; // Σημαία για έλεγχο πατήματος του κουμπιού

void setup_TCA0_high(void);
void setup_TCA0_low(void);

// Ρυθμίζουμε τον χρονιστή TCA0 high
void setup_TCA0_high() {
TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1024_gc; // Χρήση παράγοντα διαίρεσης 1024
TCA0.SPLIT.CTRLB = 0; // Απενεργοποίηση όλων των συγκριτών
TCA0.SPLIT.CTRLA |= 1;
TCA0.SPLIT.CTRLD = TCA_SPLIT_ENABLE_bm;
TCA0.SPLIT.HCMP0 = T1; // Ρυθμίζουμε τον χρονομετρητή TCA0 high με όριο το χρονικό διάστημα T1
TCA0.SPLIT.INTCTRL = TCA_SPLIT_HUNF_bm; // Ενεργοποιούμε τη διακοπή υπερχείλισης για τον χρονομετρητή TCA0 high
}

// Ρυθμίζουμε τον χρονιστή TCA0 low
void setup_TCA0_low() {
TCA0.SPLIT.CTRLA = TCA_SPLIT_CLKSEL_DIV1024_gc; // Χρήση παράγοντα διαίρεσης 1024
TCA0.SPLIT.CTRLB = 0; // Απενεργοποίηση όλων των συγκριτών
TCA0.SPLIT.CTRLA |= 1;
TCA0.SPLIT.CTRLD = TCA_SPLIT_ENABLE_bm;
TCA0.SPLIT.LCMP0 = T2_T3; // Ρυθμίζουμε τον χρονομετρητή TCA0 low με όριο τα χρονικά διαστήματα T2 = T3
TCA0.SPLIT.INTCTRL = TCA_SPLIT_LUNF_bm; // Ενεργοποιούμε τη διακοπή υπερχείλισης για τον χρονομετρητή TCA0 low
}

int main(void) {
	PORTD.DIRSET = PIN0_bm | PIN1_bm | PIN2_bm; // Ρυθμίζουμε τα pins 0, 1, 2 ως output
	PORTF.DIRCLR = PIN5_bm; // Ρυθμίζουμε το pin 5 ως input
	PORTD.OUT |= PIN1_bm | PIN0_bm; // Αρχική συνθήκη με το φανάρι των αυτοκινήτων να είναι πράσινο
	setup_TCA0_high(); // Ρύθμιση του χρονιστή TCA0
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Ενεργοποιούμε το pull-up resistor

	sei(); // Ενεργοποίηση των διακοπών γενικά
	while (x==0) {
	; //ΝΟΡ
	}
	cli();
}

// Διακοπή υπερχείλισης για τον χρονομετρητή TCA0 high (όταν έχει περάσει διάστημα T1)
ISR(TCA0_HUNF_vect) {
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;
	setup_TCA0_low(); // Ρύθμιση του χρονιστή TCA0
	PORTD.OUTCLR |= PIN0_bm | PIN1_bm;// Ανάβει πράσινο το φανάρι των πεζών και του τραμ
	PORTD.OUT |= PIN2_bm; // Ανάβουμε το κόκκινο φανάρι για τα αυτοκίνητα  
}

// Διακοπή υπερχείλισης για τον χρονομετρητή TCA0 low (όταν σταματήσει να περνάει το τραμ και έχει περάσει διάστημα T2)
ISR(TCA0_LUNF_vect) {
	int intflags = TCA0.SPLIT.INTFLAGS;
	TCA0.SPLIT.INTFLAGS=intflags;
	PORTD.OUTCLR |= PIN2_bm; // Ανάβει πράσινο το φανάρι των αυτοκινήτων
	PORTD.OUT |= PIN1_bm |PIN0_bm; // Γίνεται κόκκινο το φανάρι του τραμ και των πεζών
	button_pressed = 0; // Επαναφορά της μεταβλητής πατήματος κουμπιού
}

// Διακοπή για το πάτημα του κουμπιού των πεζών
ISR(PORTF_PORT_vect) {
	int y = PORTF.INTFLAGS;
	PORTF.INTFLAGS=y;
	PORTD.OUT |= PIN1_bm; // Το φανάρι του τραμ είναι σβηστό
	// Ελέγχουμε αν το κουμπί πεζών είναι πατημένο
	if (!(PORTF.IN & PIN5_bm)) {
		// Ρύθμιση του χρονομετρητή TCA0 low για να μετρήσει τον χρόνο T2
		setup_TCA0_low();
		PORTD.OUTCLR |= PIN0_bm;
		// Ενεργοποίηση του φαναριού πεζών
		PORTD.OUT |= PIN2_bm| PIN1_bm;
		button_pressed=1;
		// Ελέγχουμε αν το κουμπί πεζών είναι πατημένο δεύτερη φορά
		if (button_pressed==1) {
			setup_TCA0_low(); // Ρύθμιση του χρονομετρητή TCA0 low για να μετρήσει τον χρόνο T3
			PORTD.OUTCLR |= PIN2_bm; // Γίνεται πράσινο το φανάρι των αυτοκινήτων
			PORTD.OUT |= PIN0_bm; // Γίνεται κόκκινο το φανάρι των πεζών
			button_pressed = 0; // Επαναφορά της μεταβλητής πατήματος κουμπιού
		}
	}
}

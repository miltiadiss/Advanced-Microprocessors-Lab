#include <avr/io.h>
#include <avr/interrupt.h>

#define T1 1 // T1 = 1 msec
#define T2 2 // T2 = 2 msec
int total_turns = 0; // Μετρητής που αποθηκεύει το πλήθος των αριστερών στροφών που έχει πραγματοποιήσει η συσκευή
int mode = 0; // Αρχικά βρισκόμαστε σε Single-Conversion Mode

void setup_TCA0(int t){
	TCA0.SINGLE.CNT = 0;
	TCA0.SINGLE.CTRLB = 0;
	TCA0.SINGLE.CMP0 = t;
	TCA0.SINGLE.CTRLA = 0x7<<1;
	TCA0.SINGLE.CTRLA |=1;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_CMP0_bm;
}

void setup_SCM(void){
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_ENABLE_bm; // Ενεργοποίηση του ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // Ορίζεται το bit με το οποίο θα συνδεθεί ο ADC
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;    // Ενεργοποίηση Debug Mode

	//Window Comparator Mode
	ADC0.WINHT |= 10; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE = 0b00000010; //Interrupt when RESULT > WINHT
	ADC0.COMMAND |= ADC_STCONV_bm; // Αρχίζει η μετατροπή
}

void setup_FRM(void){
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_ENABLE_bm; // Ενεργοποίηση του ADC
	ADC0.CTRLA |= ADC_FREERUN_bm; // Ενεργοποιούμε το Free-Running Mode του ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // Ορίζεται το bit με το οποίο θα συνδεθεί ο ADC
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;    // Ενεργοποίηση Debug Mode

	//Window Comparator Mode
	ADC0.WINLT |= 10; //Set threshold
	ADC0.INTCTRL |= ADC_WCMP_bm; //Enable Interrupts for WCM
	ADC0.CTRLE = 0b00000001; //Interrupt when RESULT < WINLT
	ADC0.COMMAND |= ADC_STCONV_bm; // Αρχίζει η μετατροπή
}

int main(void)
{
	PORTD.DIRSET =  PIN1_bm | PIN2_bm | PIN0_bm; // Ρυθμίζουμε τα pins 0, 1, 2 ως output
	PORTD.OUT |= PIN2_bm | PIN1_bm | PIN0_bm;  // Αρχικά όλα τα pins είναι απενεργοποιημένα γιατί η συσκευή δεν έχει ξεκινήσει να κινείται

	
	setup_SCM(); // Ξεκινάμε λαμβάνοντας σήμα από τον πλάγιο αισθητήρα
	setup_TCA0(T1); // Περιμένουμε χρονικό διάστημα Τ1
	
	while (total_turns < 8) // Κάνουμε σάρωση μέχρι να ολοκληρώσουμε 8 στροφές
	{
		
		sei(); // Αρχίζουμε να δεχόμαστε σήματα διακοπής
		PORTD.OUTCLR |= PIN1_bm; // Το pin 1 της μπροστά κίνησης ενεργοποιείται για να προχωρήσει η συσκευή ευθεία
		PORTD.OUT |= PIN2_bm |PIN0_bm; // Το pin 2 της αριστερής στροφής και το pin 1 της δεξιάς στροφής  απενεργοποιείται
	}
	cli();
}

// Διακοπή για χειρισμό του ADC όταν ξεπεράσουμε το κατώφλι
ISR(ADC0_WCOMP_vect){
	// Καθαρισμός σημαίας διακοπής
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	if (mode==0){
		PORTD.OUTCLR |= PIN0_bm; // Το pin 0  ενεργοποιείται για να στρίψει η συσκευή δεξιά
		PORTD.OUT |= PIN1_bm |PIN2_bm; // Το pin 1 της μπροστά κίνησης απενεργοποιείται
		total_turns++; // Αυξάνουμε το πλήθος των στροφών που έχουμε πραγματοποιήσει κατά 1

	}
	else{
		PORTD.OUTCLR |= PIN2_bm; // Το pin 2  ενεργοποιείται για να στρίψει η συσκευή αριστερά
		PORTD.OUT |= PIN1_bm|PIN0_bm; // Το pin 1 της μπροστά κίνησης απενεργοποιείται
		total_turns++; // Αυξάνουμε το πλήθος των στροφών που έχουμε πραγματοποιήσει κατά 1
	}
	
}

// Διακοπή υπερχείλισης για τον χρονομετρητή TCA0
ISR(TCA0_CMP0_vect){
	// Καθαρισμός σημαίας διακοπής
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS=intflags;

	if (mode == 0)
	{
		// Εναλλάσσουμε τον ADC από Single-Conversion σε Free-Running Mode
		mode = 1;
		setup_FRM();
		setup_TCA0(T2); // Περιμένουμε χρονικό διάστημα Τ2

	}
	else
	{
		// Εναλλάσσουμε τον ADC από Free-Running σε Single-Conversion Mode
		mode = 0;
		setup_SCM();
		setup_TCA0(T1); // Περιμένουμε χρονικό διάστημα Τ2

	}
}

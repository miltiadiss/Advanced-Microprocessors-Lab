#include <avr/io.h>
#include <avr/interrupt.h>

int total_turns = 0; // Μετρητής που αποθηκεύει το πλήθος των αριστερών στροφών που έχει πραγματοποιήσει η συσκευή

void setup_FRM(void){
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_ENABLE_bm; // Ενεργοποίηση του ADC
	ADC0.CTRLA |= ADC_FREERUN_bm; // Ενεργοποιούμε το Free-Running Mode του ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // Ορίζεται το bit με το οποίο θα συνδεθεί ο ADC
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;    // Ενεργοποίηση Debug Mode

	// Ορίζουμε το Window-Comparator Mode
	ADC0.WINLT |= 10; // Θέτουμε το κατώφλι
	ADC0.INTCTRL |= ADC_WCMP_bm; // Ενεργοποιούμε ις διακοπές για το Window-Comparator Mode
	ADC0.CTRLE |= ADC_WINCM0_bm; // Πραγματοποίηση διακοπης όταν RES < WINLT
}

int main(void)
{
	PORTD.DIRSET = PIN1_bm | PIN2_bm; // Ρυθμίζουμε τα ΡΙΝ1 και ΡΙΝ2 ως output
	PORTD.OUT |= PIN2_bm | PIN1_bm; // Αρχικά όλα τα pins είναι απενεργοποιημένα γιατί η συσκευή δεν έχει ξεκινήσει να κινείται
	setup_FRM(); // Αρχικοποιούμε τον ADC σε Free-Running Mode

	while (total_turns < 4) // Κάνουμε σάρωση μέχρι να ολοκληρώσουμε τέσσερις αριστερές στροφές
	{
		sei(); // Αρχίζουμε να δεχόμαστε σήματα διακοπής
		PORTD.OUTCLR = PIN1_bm; // Το PIN1 ανάβει για να σηματοδοτήσει τη μπροστά κίνηση
		PORTD.OUT |= PIN2_bm; // Το PIN2 σβήνει για να εμποδίσει την αριστερή κίνηση
		ADC0.COMMAND |= ADC_STCONV_bm; // Αρχίζει η μετατροπή
	}
	cli(); // Σταματάμε να δεχόμαστε σήματα διακοπής
}

// Διακοπή για χειρισμό του ADC και αριστερή στροφή της συσκευής
ISR(ADC0_WCOMP_vect){
	// Καθαρισμός σημαίας διακοπής
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;

	PORTD.OUT |= PIN1_bm; // Σβήνουμε το PIN1 για να σταματήσει η μπροστά κίνηση
	PORTD.OUTCLR = PIN2_bm; // Ανάβουμε το ΡΙΝ2 για να σηματοδοτήσουμε την αριστερή κίνηση-στροφή
	total_turns++; // Αυξάνουμε το πλήθος των αριστερών στροφών κατά 1
}

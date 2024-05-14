#include <avr/io.h>
#include <avr/interrupt.h>

int x = 0;
int edges_count = 0; // Μετρητής που αποθηκεύει το πλήθος των ανερχόμενων ακμών
int switch_num = 0; //Μεταβλητή που καθορίζει ποιό switch πρέπει να πατηθεί
int PER = 20; // Προσομοιώνουμε Τ1 = 1 ms
int CMP = 10 ; // Προσομοιώνουμε D1 = 50% * T1


void setup_ADC_FRM(void){
	ADC0.CTRLA |= ADC_RESSEL_10BIT_gc; // 10-bit resolution
	ADC0.CTRLA |= ADC_ENABLE_bm; // Ενεργοποίηση του ADC
	ADC0.CTRLA |= ADC_FREERUN_bm; // Ενεργοποιούμε το Free-Running Mode του ADC
	ADC0.MUXPOS |= ADC_MUXPOS_AIN7_gc; // Ορίζεται το bit με το οποίο θα συνδεθεί ο ADC
	ADC0.DBGCTRL |= ADC_DBGRUN_bm;   // Ενεργοποίηση Debug Mode
	ADC0.WINLT |= 5; // Θέτουμε το κατώτερο κατώφλι
	ADC0.WINHT |= 10; // Θέτουμε το ανώτερο κατώφλι
	ADC0.INTCTRL |= ADC_RESRDY_bm;  // Ενεργοποιούμε τις διακοπές για τον RESRDY
	ADC0.COMMAND |= ADC_STCONV_bm; // Αρχίζει η μετατροπή
}

void setup_TCA0(int t){
	TCA0.SINGLE.CNT = 0; // Θέτουμε τον χρονιστή
	TCA0.SINGLE.CTRLB = 0;
	TCA0.SINGLE.CMP0 = abs(t - ADC0.RES); // Ρυθμίζουμε τον χρονιστή TCA0 με όριο το αποτέλεσμα της αφαίρεσης της τρέχουσας τιμής του RES από το κατώφλι
	TCA0.SINGLE.CTRLA = 0x7<<1; // Χρήση παράγοντα διαίρεσης 1024
	TCA0.SINGLE.CTRLA =1; // Ενεργοποιείαι ο χρονιστής
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_CMP0_bm; // Ενεργοποίηση διακοπών
}

void ventilation(void){
	TCA0.SINGLE.CTRLA=TCA_SINGLE_CLKSEL_DIV1024_gc; // Xρήση παράγοντα διαίρεσης 1024
	TCA0.SINGLE.CTRLB |= TCA_SINGLE_WGMODE_SINGLESLOPE_gc; // Ενεργοποίηση δημιουργίας κυματομορφών
	TCA0.SINGLE.PER = PER; // Oρίζουμε την μέγιστη τιμή TOP μέχρι την οοία θα μετρήσει ο παλμός
	TCA0.SINGLE.CMP0 = CMP; //Oρίζουμε τον κύκλο λειτουργίας του παλμού
	TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm; // Ενεργοποιούμε τον TCA0 και ξεκινάμε το ρολόι
	TCA0.SINGLE.INTCTRL |= TCA_SINGLE_OVF_bm; //Eνεργοποιούμε τη διακοπή που προκαλεί το OVF κατά την υπερχείλιση του χρονιστή
}

int main(void)
{
	PORTD.DIR |= PIN2_bm| PIN1_bm| PIN0_bm; // Ρυθμίζουμε τα PIN0, ΡΙΝ1 και ΡΙΝ2 ως output
	PORTD.OUT |= PIN2_bm | PIN1_bm| PIN0_bm; // Αρχικά όλα τα pins είναι απενεργοποιημένα
	PORTF.DIRCLR = PIN5_bm | PIN6_bm; // Ρυθμίζουμε τα pins 5, 6 ως input
	
	PORTF.PIN5CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Eνεργοποιούμε τον pull-up registor για το PIN5
	PORTF.PIN6CTRL |= PORT_PULLUPEN_bm | PORT_ISC_BOTHEDGES_gc; // Eνεργοποιούμε τον pull-up registor για το PIN6

	setup_ADC_FRM(); // Αρχικοποιούμε τον ADC σε Free-Running Mode
	sei(); // Αρχίζουμε να δεχόμαστε σήματα διακοπής
	while (x==0)
	{
		;
	}
	cli(); // Σταματάμε να δεχόμαστε σήματα διακοπής
}

// Διακοπή για τον έλεγχο του καταχωρητή RES σε σχέση με το κατώφλι
ISR(ADC0_RESRDY_vect){
	cli(); //Απενεργοποιούμε την δυνατότητα να ενεργοποιηθεί άλλη διακοπή

	// Καθαρισμός σημαίας διακοπής
	int intflags = ADC0.INTFLAGS;
	ADC0.INTFLAGS = intflags;
	
	if(ADC0.RES < ADC0.WINLT){
		PORTD.OUTCLR = PIN0_bm; // Ανάβουμε το ΡΙΝ0 για να σηματοδοτήσουμε την ανάγκη για πότισμα
		switch_num=5; // Θέτουμε στην μεταβλητή switch_num την τιμή 5
		ADC0.INTCTRL = 0b00000000; // Απενεργοποιούμε τις διακοπές για τον RESRDY
	}
	else if(ADC0.RES > ADC0.WINHT){
		PORTD.OUTCLR = PIN1_bm; //  Ανάβουμε το ΡΙΝ1 για να σηματοδοτήσουμε την ανάγκη για αερισμό
		switch_num=6; // Θέτουμε στην μεταβλητή switch_num την τιμή 6
		ADC0.INTCTRL = 0b00000000; // Απενεργοποιούμε τις διακοπές για τον RESRDY
	}
	else{
		PORTD.OUT |= PIN0_bm | PIN1_bm; //Σβήνουμε το ΡΙΝ0 και ΡΙΝ1
	}
}

// Διακοπή για το πάτημα των κουμπιών
ISR(PORTF_PORT_vect){
	cli(); // Απενεργοποιούμε την δυνατότητα να ενεργοποιηθεί άλλη διακοπή

	if(switch_num==5 && (PORTF.INTFLAGS & 0b00100000)== 0b00100000){ // Ελέγχουμε αν η τιμή του switch_num ισούται με 5 και αν έχει πατηθεί το PIN5 του PORTF
		setup_TCA0(ADC0.WINLT); // Αρχικοποιούμε τον TCA0
	}
	else if(switch_num==6 && (PORTF.INTFLAGS & 0b01000000)== 0b01000000){ //Ελέγχουμε αν η τιμή του switch_num ισούται με 6 και αν έχει πατηθεί το PIN6 του PORTF
		ventilation(); // Αρχικοποιούμε τον TCA0 σε PWM
	}
	else{
		PORTD.OUTCLR = PIN0_bm | PIN1_bm |PIN2_bm; // Ενεργοποιούμε τα  ΡΙΝ0, ΡΙΝ1 και  ΡΙΝ2
		PORTD.OUT |= PIN0_bm | PIN1_bm |PIN2_bm; // Ενεργοποιούμε τα  ΡΙΝ0, ΡΙΝ1 και  ΡΙΝ2
		ADC0.INTCTRL |= ADC_RESRDY_bm;  // Ενεργοποιούμε τις διακοπές για τον RESRDY
	}
	int y = PORTF.INTFLAGS; // masking
	PORTF.INTFLAGS=y;
}

// Διακοπή υπερχείλισης για τον χρονιστή TCA0
ISR(TCA0_CMP0_vect){
	cli(); // Απενεργοποιούμε την δυνατότητα να ενεργοποιηθεί άλλη διακοπή

	TCA0.SINGLE.CTRLA = 0; // Καθαρισμός TCA0
	// Καθαρισμός σημαίας διακοπής
	int intflags = TCA0.SINGLE.INTFLAGS;
	TCA0.SINGLE.INTFLAGS=intflags;

	PORTD.OUT |= PIN0_bm; // Απενεργοποιούμε το PIN0
	ADC0.INTCTRL |= ADC_RESRDY_bm; // Ενεργοποιούμε τις διακοπές για τον RESRDY
	TCA0.SINGLE.CTRLA =0; // Απενεργοποίείται ο χρονιστής
	TCA0.SINGLE.INTCTRL &= ~TCA_SINGLE_CMP0_bm;	// Απενεργοποιούμε τις διακοπές του CMP0
}

// Διακοπή για το χειρισμό ανερχόμενης ακμής από τη κυματομορφή του TCA0
ISR(TCA0_OVF_vect){
	cli(); //Απενεργοποιούμε την δυνατότητα να ενεργοποιηθεί άλλη διακοπή
	edges_count++; // Αυξάνουμε το πλήθος των ανερχόμενων ακμών κατά 1
	
	if(edges_count<=4){ // Ελέγχουμε αν το πλήθος των ανερχόμενων ακμών είναι μικρότερο ή ίσο του 4
		if(edges_count%2 == 1){ // Ελέγχουμε αν έχουμε περιττό πλήθος ανερχόμενων ακμών
			PORTD.OUTCLR = PIN2_bm; // Ενεργοποιούμε τον PIN2
		}
		else if(edges_count%2 == 0){ // Ελέγχουμε αν έχουμε άρτιο πλήθος ανερχόμενων ακμών
			PORTD.OUT |= PIN2_bm; // Απενεργοποιούμε το PIN2
		}
	}
	else{
		PORTD.OUT |= PIN2_bm| PIN1_bm; // Απενεργοποιούμε το PIN2 και PIN1
		ADC0.INTCTRL |= ADC_RESRDY_bm; // Ενεργοποιούμε τις διακοπές για τον RESRDY
		TCA0.SINGLE.CTRLB &= ~TCA_SINGLE_WGMODE_gm; //Απενεργοποίείται ο χρονιστής
		TCA0.SINGLE.INTCTRL &= ~TCA_SINGLE_OVF_bm; // Απενεργοποιούμε τις διακοπές του OVF
	}
}

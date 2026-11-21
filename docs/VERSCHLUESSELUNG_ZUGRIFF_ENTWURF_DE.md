# Verschlüsselung und Datenzugriff – Produktentwurf

## Ziel
Die Anwendung soll erkennen und erklären, wenn eine SSD zwar technisch erreichbar ist, ihre Daten aber durch Verschlüsselung oder gesperrte Volumes nicht lesbar sind.

## Erkennung
- Hinweise auf BitLocker, LUKS und weitere erkennbare Volume-Verschlüsselungen anzeigen.
- Hardwareseitige Sperren und softwareseitige Verschlüsselung soweit möglich voneinander unterscheiden.
- Erklären, dass eine erfolgreiche Controllerdiagnose keine Entschlüsselung bedeutet.
- Nicht identifizierbare Verfahren als unbekannt markieren.

## Benutzerführung
- Status „verschlüsselt“, „gesperrt“, „entsperrt“ oder „nicht eindeutig“ anzeigen, soweit begründbar.
- Erklären, welche vorhandenen Wiederherstellungsschlüssel oder Berechtigungen für einen legitimen Zugriff erforderlich sein können.
- Bereitgestellte Schlüssel nur nach ausdrücklicher Benutzerentscheidung für den vorgesehenen Zugriff verwenden.
- Keine automatische Übermittlung von Schlüsseln an Online-KI oder externe Dienste.
- Sensible Schlüssel nicht in Diagnoseberichten, Screenshots oder Fallakten im Klartext aufnehmen.

## Datenrettung
- Vorhandene Verschlüsselung bei der Planung eines Datenträgerabbilds berücksichtigen.
- Unterscheiden zwischen einem verschlüsselten Rohabbild und tatsächlich zugänglichen Dateien.
- Nach dem Entsperren nachvollziehbar anzeigen, welche Daten zugänglich wurden.
- Keine unrealistischen Zusagen über die Umgehung moderner Verschlüsselung.

## Noch gewünschte Funktionen
Verschlüsselungsstatus, verständliche Zugriffshilfe, sichere Schlüsselbehandlung und Integration in Fallakten.

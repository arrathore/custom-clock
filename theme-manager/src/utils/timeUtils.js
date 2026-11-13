export function formatTime(time, use24Hour) {
  if (use24Hour) {
    return time;
  }

  const [hours, minutes] = time.split(":");
  const hour = Number(hours);

  const displayHour = hour % 12 || 12;
  const period = hour < 12 ? "AM" : "PM";

  return `${String(displayHour).padStart(2, "0")}:${minutes} ${period}`;
}

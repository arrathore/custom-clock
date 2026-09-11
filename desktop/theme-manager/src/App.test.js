import { render, screen } from '@testing-library/react';
import App from './App';

test('renders theme manager heading', () => {
  render(<App />);
  const headingElement = screen.getByText(/Theme Manager/i);
  expect(headingElement).toBeInTheDocument();
});

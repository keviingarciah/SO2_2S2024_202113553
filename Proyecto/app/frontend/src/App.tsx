import { BrowserRouter, Routes, Route } from "react-router-dom";

import LandingPage from "./pages/LandingPage";
import MemoryUsePage from "./pages/MemoryUsePage";
import MemoryTimePage from "./pages/MemoryTimePage";
import PagesPage from "./pages/PagesPage";
import FaultsPage from "./pages/FaultsPage";
import ProcessesPage from "./pages/ProcessesPage";

import NavBar from "./components/NavBar";
import Footer from "./components/Footer";

function App() {
  return (
    <BrowserRouter>
      <div className="flex flex-col min-h-screen bg-gray-100">
        <NavBar />
        <main className="container mx-auto px-4 flex-grow ">
          <Routes>
            <Route path="/" element={<LandingPage />} />
            <Route path="/memory-use" element={<MemoryUsePage />} />
            <Route path="/memory-time" element={<MemoryTimePage />} />
            <Route path="/faults" element={<FaultsPage />} />
            <Route path="/pages" element={<PagesPage />} />
            <Route path="/processes" element={<ProcessesPage />} />
          </Routes>
        </main>
        <Footer />
      </div>
    </BrowserRouter>
  );
}

export default App;

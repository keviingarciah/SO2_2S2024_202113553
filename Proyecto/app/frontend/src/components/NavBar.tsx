import { NavLink } from "react-router-dom";

function Navbar() {
  return (
    <div className="bg-blue-600">
      <nav className="flex items-center justify-between mx-5 p-4">
        <p className="text-white font-semibold text-4xl">SO2</p>
        <div className="flex">
          <NavLink
            to="/memory-use"
            className="ml-auto text-white text-xl font-semibold hover:bg-blue-700 px-4 py-4 rounded transition duration-300 ease-in-out"
          >
            Uso de Memoria
          </NavLink>
          <NavLink
            to="/memory-time"
            className="ml-auto text-white text-xl font-semibold hover:bg-blue-700 px-4 py-4 rounded transition duration-300 ease-in-out"
          >
            Memoria Histórica
          </NavLink>
          <NavLink
            to="/faults"
            className="ml-auto text-white text-xl font-semibold hover:bg-blue-700 px-4 py-4 rounded transition duration-300 ease-in-out"
          >
            Fallos de Página
          </NavLink>
          <NavLink
            to="/pages"
            className="ml-auto text-white text-xl font-semibold hover:bg-blue-700 px-4 py-4 rounded transition duration-300 ease-in-out"
          >
            Páginas
          </NavLink>
          <NavLink
            to="/processes"
            className="ml-auto text-white text-xl font-semibold hover:bg-blue-700 px-4 py-4 rounded transition duration-300 ease-in-out"
          >
            Procesos
          </NavLink>
        </div>
      </nav>
    </div>
  );
}

export default Navbar;

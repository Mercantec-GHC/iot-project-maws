FROM mcr.microsoft.com/dotnet/aspnet:8.0 AS base
WORKDIR /app
EXPOSE 8080

FROM mcr.microsoft.com/dotnet/sdk:8.0 AS build
ARG BUILD_CONFIGURATION=Release
WORKDIR /src

# Copy project files and restore dependencies
COPY ["API/API.csproj", "API/"]
COPY ["DomainModels/DomainModels.csproj", "DomainModels/"] 
RUN dotnet restore "API/API.csproj"

# Copy all source code
COPY . .
WORKDIR "/src/API"

# Build and publish in the same stage
RUN dotnet build "API.csproj" -c $BUILD_CONFIGURATION -o /app/build
RUN dotnet publish "API.csproj" -c $BUILD_CONFIGURATION -o /app/publish /p:UseAppHost=false

FROM base AS final
WORKDIR /app
COPY --from=build /app/publish .

# Set environment variables for production
ENV ASPNETCORE_ENVIRONMENT=Production
ENV ASPNETCORE_URLS=http://+:8080

ENTRYPOINT ["dotnet", "API.dll"]